// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockadeLabsSkyboxAi.h"
#include "BlockadeLabsSkyboxAiStyle.h"
#include "BlockadeLabsSkyboxAiCommands.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "ISettingsModule.h"
#include "MessageLogModule.h"
#include "SSkyboxAiWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "ToolMenus.h"
#include "SkyboxAi/SKyboxAiHttpClient.h"

static const FName BlockadeLabsSkyboxAiTabName("BlockadeLabs SkyboxAI");

#define LOCTEXT_NAMESPACE "BlockadeLabsSkyboxAiModule"

void FBlockadeLabsSkyboxAiModule::StartupModule()
{
  FBlockadeLabsSkyboxAiStyle::Initialize();
  FBlockadeLabsSkyboxAiStyle::ReloadTextures();

  FBlockadeLabsSkyboxAiCommands::Register();

  SkyboxWidgetData = MakeShareable(new FSkyboxAiWidgetData);

  PluginCommands = MakeShareable(new FUICommandList);

  PluginCommands->MapAction(
    FBlockadeLabsSkyboxAiCommands::Get().OpenPluginWindow,
    FExecuteAction::CreateRaw(this, &FBlockadeLabsSkyboxAiModule::OpenPluginMenuItemClicked),
    FCanExecuteAction()
    );

  UToolMenus::RegisterStartupCallback(
    FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlockadeLabsSkyboxAiModule::RegisterMenus)
    );

  FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                            BlockadeLabsSkyboxAiTabName,
                            FOnSpawnTab::CreateRaw(this, &FBlockadeLabsSkyboxAiModule::OnSpawnPluginTab)
                            )
                          .SetDisplayName(LOCTEXT("BlockadeLabsSkyboxAiTabTitle", "Blockade Labs SkyboxAI"))
                          .SetMenuType(ETabSpawnerMenuType::Hidden);

  if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
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

  FMessageLogModule &MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
  FMessageLogInitializationOptions InitOptions;
  InitOptions.bShowPages = true;
  InitOptions.bAllowClear = true;
  InitOptions.bShowFilters = true;


  MessageLogModule.RegisterLogListing(
    "SkyboxAI",
    FText::Format(
      NSLOCTEXT("{0}", "SkyboxAiAPILogLabel", "Blockade Labs SkyboxAI"),
      FText::FromName(SkyboxAiHttpClient::GMessageLogName)
      ),
    InitOptions
    );
}

void FBlockadeLabsSkyboxAiModule::ShutdownModule()
{
  UToolMenus::UnRegisterStartupCallback(this);

  UToolMenus::UnregisterOwner(this);

  FBlockadeLabsSkyboxAiStyle::Shutdown();

  FBlockadeLabsSkyboxAiCommands::Unregister();

  FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BlockadeLabsSkyboxAiTabName);

  if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->UnregisterSettings("Engine", "Plugins", "Blockade Labs");
  }

  if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
  {
    FMessageLogModule &MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
    MessageLogModule.UnregisterLogListing("SkyboxAI");
  }
}

void FBlockadeLabsSkyboxAiModule::OpenPluginMenuItemClicked()
{
  FGlobalTabmanager::Get()->TryInvokeTab(BlockadeLabsSkyboxAiTabName);
}

TSharedRef<SDockTab> FBlockadeLabsSkyboxAiModule::OnSpawnPluginTab(const FSpawnTabArgs &SpawnTabArgs)
{
  return SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    [GetSkyboxWidget()];
}

void FBlockadeLabsSkyboxAiModule::RegisterMenus()
{
  FToolMenuOwnerScoped OwnerScoped(this);

  {
    UToolMenu *Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
    {
      FToolMenuSection &Section = Menu->FindOrAddSection("BlockadeLabs");
      Section.AddMenuEntryWithCommandList(
        FBlockadeLabsSkyboxAiCommands::Get().OpenPluginWindow,
        PluginCommands,
        FText::FromString("Blockade Labs SkyboxAI"),
        FText::FromString("Generate a Skybox using Blockade Labs SkyboxAI and export it to the Content folder")
        );
    }
  }

  {
    UToolMenu *ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
    {
      FToolMenuSection &Section = ToolbarMenu->FindOrAddSection("Settings");
      {
        FToolMenuEntry &Entry = Section.AddEntry(
          FToolMenuEntry::InitToolBarButton(FBlockadeLabsSkyboxAiCommands::Get().OpenPluginWindow)
          );
        Entry.SetCommandList(PluginCommands);
      }
    }
  }
}

TSharedRef<SSkyboxAiWidget> FBlockadeLabsSkyboxAiModule::GetSkyboxWidget()
{
  return SAssignNew(SkyboxWidget, SSkyboxAiWidget)
    .Prompt(SkyboxWidgetData->Prompt)
    .NegativeText(SkyboxWidgetData->NegativeText)
    .Category(SkyboxWidgetData->Category)
    .ExportType(SkyboxWidgetData->ExportType)
    .bEnrichPrompt(SkyboxWidgetData->bEnrichPrompt)
    .OnSkyboxAiWidgetDataChanged_Raw(this, &FBlockadeLabsSkyboxAiModule::OnSkyboxAiWidgetDataChanged);
}

void FBlockadeLabsSkyboxAiModule::OnSkyboxAiWidgetDataChanged(const FSkyboxAiWidgetData &Data)
{
  SkyboxWidgetData->Category = Data.Category;
  SkyboxWidgetData->ExportType = Data.ExportType;
  SkyboxWidgetData->bEnrichPrompt = Data.bEnrichPrompt;
  SkyboxWidgetData->Prompt = Data.Prompt;
  SkyboxWidgetData->NegativeText = Data.NegativeText;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlockadeLabsSkyboxAiModule, BlockadeLabsSkyboxAi)
