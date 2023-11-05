// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockadeLabs_SkyboxAI.h"
#include "BlockadeLabs_SkyboxAIStyle.h"
#include "BlockadeLabs_SkyboxAICommands.h"
#include "BlockadeLabs_SkyboxAISettings.h"
#include "ISettingsModule.h"
#include "MessageLogModule.h"
#include "SSkyboxAIWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "ToolMenus.h"
#include "SkyboxAI/SKyboxAiHttpClient.h"

static const FName BlockadeLabs_SkyboxAITabName("BlockadeLabs SkyboxAI");

#define LOCTEXT_NAMESPACE "FBlockadeLabs_SkyboxAIModule"

void FBlockadeLabs_SkyboxAIModule::StartupModule()
{
  FBlockadeLabs_SkyboxAIStyle::Initialize();
  FBlockadeLabs_SkyboxAIStyle::ReloadTextures();

  FBlockadeLabs_SkyboxAICommands::Register();

  SkyboxWidgetData = MakeShareable(new FSkyboxAIWidgetData);

  PluginCommands = MakeShareable(new FUICommandList);

  PluginCommands->MapAction(
    FBlockadeLabs_SkyboxAICommands::Get().OpenPluginWindow,
    FExecuteAction::CreateRaw(this, &FBlockadeLabs_SkyboxAIModule::OpenPluginMenuItemClicked),
    FCanExecuteAction()
    );

  UToolMenus::RegisterStartupCallback(
    FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlockadeLabs_SkyboxAIModule::RegisterMenus)
    );

  FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                            BlockadeLabs_SkyboxAITabName,
                            FOnSpawnTab::CreateRaw(this, &FBlockadeLabs_SkyboxAIModule::OnSpawnPluginTab)
                            )
                          .SetDisplayName(LOCTEXT("FBlockadeLabs_SkyboxAITabTitle", "BlockadeLabs SkyboxAI"))
                          .SetMenuType(ETabSpawnerMenuType::Hidden);

  if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->RegisterSettings(
      "Editor",
      "Plugins",
      "SkyboxAI",
      LOCTEXT("RuntimeSettingsName", "Blockade Labs SkyboxAI"),
      LOCTEXT("RuntimeSettingsDescription", "API Settings for Skybox AI API"),
      GetMutableDefault<UBlockadeLabs_SkyboxAISettings>()
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
      NSLOCTEXT("{0}", "SkyboxAIAPILogLabel", "Blockade Labs SkyboxAI"),
      FText::FromName(SkyboxAiHttpClient::GMessageLogName)
      ),
    InitOptions
    );
}

void FBlockadeLabs_SkyboxAIModule::ShutdownModule()
{
  UToolMenus::UnRegisterStartupCallback(this);

  UToolMenus::UnregisterOwner(this);

  FBlockadeLabs_SkyboxAIStyle::Shutdown();

  FBlockadeLabs_SkyboxAICommands::Unregister();

  FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BlockadeLabs_SkyboxAITabName);

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

void FBlockadeLabs_SkyboxAIModule::OpenPluginMenuItemClicked()
{
  FGlobalTabmanager::Get()->TryInvokeTab(BlockadeLabs_SkyboxAITabName);
}

TSharedRef<SDockTab> FBlockadeLabs_SkyboxAIModule::OnSpawnPluginTab(const FSpawnTabArgs &SpawnTabArgs)
{
  return SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    [GetSkyboxWidget()];
}

void FBlockadeLabs_SkyboxAIModule::RegisterMenus()
{
  FToolMenuOwnerScoped OwnerScoped(this);

  {
    UToolMenu *Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
    {
      FToolMenuSection &Section = Menu->FindOrAddSection("BlockadeLabs");
      Section.AddMenuEntryWithCommandList(FBlockadeLabs_SkyboxAICommands::Get().OpenPluginWindow, PluginCommands);
    }
  }

  {
    UToolMenu *ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
    {
      FToolMenuSection &Section = ToolbarMenu->FindOrAddSection("Settings");
      {
        FToolMenuEntry &Entry = Section.AddEntry(
          FToolMenuEntry::InitToolBarButton(FBlockadeLabs_SkyboxAICommands::Get().OpenPluginWindow)
          );
        Entry.SetCommandList(PluginCommands);
      }
    }
  }
}

TSharedRef<SSkyboxAIWidget> FBlockadeLabs_SkyboxAIModule::GetSkyboxWidget()
{
  return SAssignNew(SkyboxWidget, SSkyboxAIWidget)
    .Prompt(SkyboxWidgetData->Prompt)
    .NegativeText(SkyboxWidgetData->NegativeText)
    .Category(SkyboxWidgetData->Category)
    .ExportType(SkyboxWidgetData->ExportType)
    .bEnrichPrompt(SkyboxWidgetData->bEnrichPrompt)
    .OnSkyboxAIWidgetDataChanged_Raw(this, &FBlockadeLabs_SkyboxAIModule::OnSkyboxAIWidgetDataChanged);
}

void FBlockadeLabs_SkyboxAIModule::OnSkyboxAIWidgetDataChanged(const FSkyboxAIWidgetData &Data)
{
  SkyboxWidgetData->Category = Data.Category;
  SkyboxWidgetData->ExportType = Data.ExportType;
  SkyboxWidgetData->bEnrichPrompt = Data.bEnrichPrompt;
  SkyboxWidgetData->Prompt = Data.Prompt;
  SkyboxWidgetData->NegativeText = Data.NegativeText;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlockadeLabs_SkyboxAIModule, BlockadeLabs_SkyboxAI)
