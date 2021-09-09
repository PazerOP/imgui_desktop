#pragma once

#include <imgui.h>
#include <mh/memory/stack_info.hpp>
#include <mh/types/disable_copy_move.hpp>

namespace ImGuiDesktop
{
	namespace detail
	{
		template<typename T> struct StorageHelperBase;

		template<typename T>
		struct StorageHelperBase<T*> : mh::disable_copy_move
		{
		protected:
			static constexpr auto GET_FUNC = &ImGuiStorage::GetVoidPtr;
			static constexpr auto SET_FUNC = &ImGuiStorage::SetVoidPtr;
		};

		template<>
		struct StorageHelperBase<float> : mh::disable_copy_move
		{
		protected:
			static constexpr auto GET_FUNC = &ImGuiStorage::GetFloat;
			static constexpr auto SET_FUNC = &ImGuiStorage::SetFloat;
		};

		template<>
		struct StorageHelperBase<int32_t> : mh::disable_copy_move
		{
		protected:
			static constexpr auto GET_FUNC = &ImGuiStorage::GetInt;
			static constexpr auto SET_FUNC = &ImGuiStorage::SetInt;
		};
		template<> struct StorageHelperBase<uint32_t> : StorageHelperBase<int32_t> {};
		template<> struct StorageHelperBase<int64_t> : StorageHelperBase<int32_t> {};
		template<> struct StorageHelperBase<uint64_t> : StorageHelperBase<int32_t> {};

		template<>
		struct StorageHelperBase<bool> : mh::disable_copy_move
		{
		protected:
			static constexpr auto GET_FUNC = &ImGuiStorage::GetBool;
			static constexpr auto SET_FUNC = &ImGuiStorage::SetBool;
		};

		union SplitInt64
		{
			SplitInt64() = default;
			constexpr SplitInt64(uint64_t uVal) : u64(uVal) {}
			constexpr SplitInt64(int64_t iVal) : i64(iVal) {}

			uint64_t& GetValue(uint64_t& val) { val = u64; return u64; }
			int64_t& GetValue(int64_t& val) { val = i64; return i64; }

			uint64_t u64;
			int64_t i64;
			struct
			{
				uint32_t uLower;
				uint32_t uUpper;
			};
			struct
			{
				int32_t iLower;
				int32_t iUpper;
			};
		};
	}

	// *** THIS MUST BE STATICALLY ALLOCATED ***
	// TODO: Use platform APIs to make sure this isn't allocated on the stack
	template<typename T>
	struct Storage final : detail::StorageHelperBase<T>
	{
	private:
		using base_t = detail::StorageHelperBase<T>;

		static constexpr bool IS_INT64 = std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>;
		static constexpr size_t ID_COUNT = IS_INT64 ? 2 : 1;

		using IDArray = std::array<ImGuiID, ID_COUNT>;

	public:
		Storage()
		{
			assert(!mh::is_variable_on_current_stack(this));
		}

		Storage(const Storage<T>&) = delete;
		Storage(Storage<T>&&) = delete;

		Storage<T>& operator=(const Storage<T>&) = delete;
		Storage<T>& operator=(Storage<T>&&) = delete;

		struct Scope
		{
			[[nodiscard]] T Get(T defaultVal = {}) const
			{
				constexpr auto getter = base_t::GET_FUNC;

				T value;
				if constexpr (IS_INT64)
				{
					detail::SplitInt64 splitVal;
					detail::SplitInt64 splitDefaultVal(defaultVal);

					splitVal.iLower = (m_Storage->*getter)(m_IDs[0], splitDefaultVal.iLower);
					splitVal.iUpper = (m_Storage->*getter)(m_IDs[1], splitDefaultVal.iUpper);

					splitVal.GetValue(value);
				}
				else
				{
					static_assert(decltype(m_IDs){}.size() == 1);
					value = (m_Storage->*getter)(m_IDs[0], defaultVal);
				}
#ifdef _DEBUG
				m_LastValue = value;
#endif
				return value;
			}

			void Set(T value) const
			{
				constexpr auto setter = base_t::SET_FUNC;

				if constexpr (IS_INT64)
				{
					detail::SplitInt64 splitVal(value);

					(m_Storage->*setter)(m_IDs[0], splitVal.iLower);  // lower
					(m_Storage->*setter)(m_IDs[1], splitVal.iUpper);  // upper
				}
				else
				{
					static_assert(decltype(m_IDs){}.size() == 1);
					(m_Storage->*setter)(m_IDs[0], value);
				}

#ifdef _DEBUG
				m_LastValue = value;
#endif
			}

			operator T() const { return Get(); }
			const Scope& operator=(T value) const
			{
				Set(std::move(value));
				return *this;
			}

		private:
			friend struct Storage<T>;
			constexpr Scope(ImGuiStorage* storage, const IDArray& ids) : m_Storage(storage), m_IDs(ids) {}

#ifdef _DEBUG
			mutable T m_LastValue{};
#endif

			ImGuiStorage* m_Storage{};
			IDArray m_IDs{};
		};

		[[nodiscard]] T Get(T defaultVal = {}) const { return Snapshot().Get(std::move(defaultVal)); }
		void Set(T value) const { return Snapshot().Set(std::move(value)); }

		Scope Snapshot() const
		{
			return Scope(ImGui::GetStateStorage(), GetIDs());
		}

	private:
		char m_IDAddresses[ID_COUNT];

		IDArray GetIDs() const
		{
			std::array<ImGuiID, ID_COUNT> retVal;

			for (size_t i = 0; i < ID_COUNT; i++)
				retVal[i] = ImGui::GetID((const void*)(&m_IDAddresses[i]));

			return retVal;
		}
	};
}
