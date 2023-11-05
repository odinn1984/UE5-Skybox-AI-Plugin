// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BlockadeLabs_SkyboxAIStyle.h"

class FBlockadeLabs_SkyboxAICommands : public TCommands<FBlockadeLabs_SkyboxAICommands>
{
public:
  FBlockadeLabs_SkyboxAICommands()
    : TCommands<FBlockadeLabs_SkyboxAICommands>(
      TEXT("BlockadeLabs_SkyboxAI"),
      NSLOCTEXT("Contexts", "BlockadeLabs_SkyboxAI", "BlockadeLabs SkyboxAI Plugin"),
      NAME_None,
      FBlockadeLabs_SkyboxAIStyle::GetStyleSetName()
      )
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override;

public:
  TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
