#include "SkyboxAiApiModule.h"
#include "ISettingsModule.h"
#include "MessageLogModule.h"
#include "SKyboxAiHttpClient.h"
#include "BlockadeLabsSkyboxAiSettings.h"

#define LOCTEXT_NAMESPACE "SkyboxAiApiModule"

void FSkyboxAiApiModule::StartupModule()
{
  if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->RegisterSettings(
      "Editor",
      "Plugins",
      "SkyboxAI",
      LOCTEXT("RuntimeSettingsName", "Blockade Labs SkyboxAI"),
      LOCTEXT("RuntimeSettingsDescription", "API Settings for Skybox AI API"),
      GetMutableDefault<UBlockadeLabsSkyboxAiSettings>()
    );
  }

  FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
  FMessageLogInitializationOptions InitOptions;
  InitOptions.bShowPages = true;
  InitOptions.bAllowClear = true;
  InitOptions.bShowFilters = true;


  MessageLogModule.RegisterLogListing(
    "SkyboxAI",
    FText::Format(
      NSLOCTEXT("{0}", "SkyboxAiAPILogLabel", "Blockade Labs SkyboxAI"),
      FText::FromName(SkyboxAiHttpClientDefinitions::GMessageLogName)
    ),
    InitOptions
  );
}

void FSkyboxAiApiModule::ShutdownModule()
{
  if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->UnregisterSettings("Engine", "Plugins", "Blockade Labs");
  }

  if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
  {
    FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
    MessageLogModule.UnregisterLogListing("SkyboxAI");
  }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSkyboxAiApiModule, SkyboxAiApi);
