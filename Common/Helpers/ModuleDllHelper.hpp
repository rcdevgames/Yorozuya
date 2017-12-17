#pragma once

#include <Windows.h>
#include <filesystem>

#include "../Common/Interfaces/ModuleInterface.h"

namespace ModuleDllHelper
{
    namespace fs = ::std::experimental::filesystem::v1;
    using namespace Yorozuya;
    
    class CModuleDll : public Module::IModule
    {
    public:
        CModuleDll(const fs::path& dll_path)
        {
            m_hDll = LoadLibraryW(dll_path.generic_wstring().c_str());
            assert(m_hDll != NULL);

            m_fnCreateModule = (ModuleApi::CreateModule_ptr)GetProcAddress(m_hDll, ModuleApi::csNameCreateModule);
            m_fnReleaseModule = (ModuleApi::ReleaseModule_ptr)GetProcAddress(m_hDll, ModuleApi::csNameReleaseModule);

            m_impl = std::move(
                Module::Module_ptr(
                    m_fnCreateModule(), 
                    [&](Module::IModule* obj) { 
                        m_fnReleaseModule(obj); 
                    }));
        };

        virtual ~CModuleDll()
        {
            m_impl.reset();
            CloseHandle(m_hDll);
        }

        virtual void load()
        {
            m_impl->load();
        }

        virtual void unload()
        {
            m_impl->unload();
        }

        virtual void loop()
        {
            m_impl->loop();
        }

        virtual Module::ModuleVersion_t get_version()
        {
            return m_impl->get_version();
        }

        virtual Module::ModuleName_t get_name()
        {
            return m_impl->get_name();
        }

        virtual void configure(const rapidjson::Value& nodeConfig)
        {
            m_impl->configure(nodeConfig);
        }

    private:
        HMODULE m_hDll;
        Module::Module_ptr m_impl;
        ModuleApi::CreateModule_ptr m_fnCreateModule;
        ModuleApi::ReleaseModule_ptr m_fnReleaseModule;
    };
}
