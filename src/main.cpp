namespace Hooks
{
	enum class SAVEFILE_CATEGORY : std::int32_t
	{
		kUser = 0x0,
		kAuto = 0x1,
		kQuick = 0x2,
		kExit = 0x3
	};

	class hkQuickLoad
	{
	private:
		static void QueueSaveLoadTask(RE::BGSSaveLoadManager* a_this, RE::BGSSaveLoadManager::QUEUED_TASK a_task)
		{
			if (a_task == RE::BGSSaveLoadManager::QUEUED_TASK::kQuickLoad) {
				return _QueueSaveLoadTask(a_this, RE::BGSSaveLoadManager::QUEUED_TASK::kLoadMostRecentSave);
			}

			return _QueueSaveLoadTask(a_this, a_task);
		}

		static inline REL::Relocation<decltype(QueueSaveLoadTask)> _QueueSaveLoadTask;

	public:
		static void Install()
		{
			auto& trampoline = REL::GetTrampoline();
			_QueueSaveLoadTask = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID{ 1470086, 2249427 }, REL::Offset{ 0xC3, 0xC3 } }.address(), & QueueSaveLoadTask);
		}
	};

	class hkQuickSave
	{
	private:
		static void GenerateSaveFileName(RE::BGSSaveLoadManager* a_this, char* a_saveFileName, bool a_displayOnly, [[maybe_unused]] SAVEFILE_CATEGORY a_saveCategory)
		{
			a_this->quicksaveFileName = RE::BSFixedString("");
			return _GenerateSaveFileName(a_this, a_saveFileName, a_displayOnly, SAVEFILE_CATEGORY::kUser);
		}

		static inline REL::Relocation<decltype(GenerateSaveFileName)> _GenerateSaveFileName;

	public:
		static void Install()
		{
			auto& trampoline = REL::GetTrampoline();

			REL::Relocation<uintptr_t> target{ REL::ID{ 1055666, 2228083 }, REL::Offset{ 0x5D, 0x20C } };
			target.write({ 0x01 });

			_GenerateSaveFileName = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID{ 1055666, 2228083 }, REL::Offset{ 0x32, 0x1ED } }.address(), & GenerateSaveFileName);
		}
	};
}

F4SE_PLUGIN_QUERY(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
	if (const auto data = F4SE::PluginVersionData::GetSingleton()) {
		a_info->infoVersion = F4SE::PluginInfo::kVersion;
		a_info->name = data->GetPluginName().data();
		a_info->version = data->GetPluginVersion().pack();
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < REL::Version(F4SE::RUNTIME_1_10_163)) {
		return false;
	}

	return true;
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .trampoline = true, .trampolineSize = 32 });

	Hooks::hkQuickLoad::Install();
	Hooks::hkQuickSave::Install();

	REX::INFO("Loaded"sv);

	return true;
}
