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
			auto& trampoline = F4SE::GetTrampoline();
			switch (REL::Module::get().GetRuntime()) {
				case REL::Module::Runtime::NG:
					_QueueSaveLoadTask = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID(2249427), 0xC3 }.address(), &QueueSaveLoadTask);
					break;
				default:
					_QueueSaveLoadTask = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID(1470086), 0xC3 }.address(), &QueueSaveLoadTask);
					break;
			}
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
			auto& trampoline = F4SE::GetTrampoline();
			switch (REL::Module::get().GetRuntime()) {
				case REL::Module::Runtime::NG:
					_GenerateSaveFileName = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID(2228083), 0x1ED }.address(), &GenerateSaveFileName);
					break;
				default:
					_GenerateSaveFileName = trampoline.write_call<5>(REL::Relocation<uintptr_t>{ REL::ID(1055666), 0x32 }.address(), &GenerateSaveFileName);
					break;
			}
		}
	};
}


F4SE_EXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	auto data = F4SE::PluginVersionData();

	data.AuthorName(Plugin::AUTHOR);
	data.PluginName(Plugin::NAME);
	data.PluginVersion(Plugin::VERSION);

	data.UsesAddressLibrary(true);
	data.IsLayoutDependent(true);
	data.UsesSigScanning(false);
	data.HasNoStructUse(false);

	data.CompatibleVersions({ F4SE::RUNTIME_LATEST_OG, F4SE::RUNTIME_LATEST_NG });
	return data;
}();

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION.major();

	if (a_f4se->IsEditor()) {
		logger::critical("Loading in editor is not supported"sv);
		return false;
	}

	const auto version = a_f4se->RuntimeVersion();
	if (version < REL::Relocate(F4SE::RUNTIME_LATEST_OG, F4SE::RUNTIME_LATEST_NG)) {
		logger::critical("Unsupported runtime v{}"sv, version.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

	F4SE::AllocTrampoline(32);
	Hooks::hkQuickLoad::Install();
	Hooks::hkQuickSave::Install();

	logger::info("Loaded"sv);

	return true;
}
