// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SSkyboxAiWidget.h"
#include "Modules/ModuleManager.h"

class FBlockadeLabsSkyboxAiModule : public IModuleInterface
{
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  TSharedPtr<FUICommandList> PluginCommands;
  TSharedPtr<SSkyboxAiWidget> SkyboxWidget;
  TSharedPtr<FSkyboxAiWidgetData> SkyboxWidgetData;

  void OpenPluginMenuItemClicked();
  TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs &SpawnTabArgs);

  void RegisterMenus();
  TSharedRef<SSkyboxAiWidget> GetSkyboxWidget();

  void OnSkyboxAiWidgetDataChanged(const FSkyboxAiWidgetData &Data);
};
