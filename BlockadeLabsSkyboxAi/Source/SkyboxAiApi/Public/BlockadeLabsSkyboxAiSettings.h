#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BlockadeLabsSkyboxAiSettings.generated.h"

UCLASS(config = "BlockdaLabs", meta = (DisplayName = "Blockade Labs Skybox AI"))
class SKYBOXAIAPI_API UBlockadeLabsSkyboxAiSettings : public UDeveloperSettings
{
  GENERATED_BODY()

public:
  UPROPERTY(config, EditAnywhere, Category = "General Settings")
  FString SaveDirectory = TEXT("/Game/Content/SkyboxAI/Exports");

  UPROPERTY(config, EditAnywhere, Category = "General Settings")
  bool bEnablePremiumContent = true;

  UPROPERTY(config, EditAnywhere, Category = "API Settings")
  FString ApiKey = TEXT("");

  UPROPERTY(config, EditAnywhere, Category = "API Settings", AdvancedDisplay)
  FString ApiEndpoint = TEXT("https://backend.blockadelabs.com/api/v1");

  UPROPERTY(
    config,
    EditAnywhere,
    Category = "API Settings",
    AdvancedDisplay,
    meta = (
      UIMin = "0.25",
      UIMax = "60.0",
      ClampMin = "0.25",
      ClampMax = "60.0",
      DisplayName = "API Polling Interval In Seconds"
    )
  )
  float ApiPollingInterval = 1.0f;
};
