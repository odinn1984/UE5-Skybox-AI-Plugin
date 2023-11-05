// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SSkyboxAIWidget.h"
#include "Modules/ModuleManager.h"

class FBlockadeLabs_SkyboxAIModule : public IModuleInterface
{
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  TSharedPtr<FUICommandList> PluginCommands;
  TSharedPtr<SSkyboxAIWidget> SkyboxWidget;
  TSharedPtr<FSkyboxAIWidgetData> SkyboxWidgetData;

  void OpenPluginMenuItemClicked();
  TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs &SpawnTabArgs);

  void RegisterMenus();
  TSharedRef<SSkyboxAIWidget> GetSkyboxWidget();

  void OnSkyboxAIWidgetDataChanged(const FSkyboxAIWidgetData &Data);
};
