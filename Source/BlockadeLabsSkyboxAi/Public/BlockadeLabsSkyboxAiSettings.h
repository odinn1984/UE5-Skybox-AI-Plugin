#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BlockadeLabsSkyboxAiSettings.generated.h"

UCLASS(config = "BlackdaleLabs", meta = (DisplayName = "Blockade Labs Skybox AI"))
class UBlockadeLabsSkyboxAiSettings : public UDeveloperSettings
{
  GENERATED_BODY()

public:
  UPROPERTY(config, EditAnywhere, Category = "General Settings")
  FString SaveDirectory = TEXT("/Game/Content/SkyboxAI/Exports");

  UPROPERTY(config, EditAnywhere, Category = "General Settings")
  bool bEnablePremiumContent = true;

  UPROPERTY(config, EditAnywhere, Category = "API Settings")
  FString ApiKey;

  UPROPERTY(config, EditAnywhere, Category = "API Settings", AdvancedDisplay)
  FString ApiEndpoint = TEXT("https://backend.blockadelabs.com/api/v1");

  UPROPERTY(
    config,
    EditAnywhere,
    Category = "API Settings",
    AdvancedDisplay,
    meta = ( UIMin = "0.1", UIMax = "60.0", ClampMin = "0.1", ClampMax = "60.0")
  )
  float ApiPollingInterval = 5.0f;

  UBlockadeLabsSkyboxAiSettings(const FObjectInitializer &ObjectInitializer);
};
