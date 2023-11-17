// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BlockadeLabsSkyboxAiStyle.h"

class FBlockadeLabsSkyboxAiCommands : public TCommands<FBlockadeLabsSkyboxAiCommands>
{
public:
  FBlockadeLabsSkyboxAiCommands()
    : TCommands<FBlockadeLabsSkyboxAiCommands>(
      TEXT("BlockadeLabsSkyboxAi"),
      NSLOCTEXT("Contexts", "BlockadeLabsSkyboxAi", "BlockadeLabs SkyboxAI Plugin"),
      NAME_None,
      FBlockadeLabsSkyboxAiStyle::GetStyleSetName()
    )
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override;

public:
  TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
