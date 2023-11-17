#pragma once

#include "CoreMinimal.h"
#include "SKyboxAiHttpClient.h"
#include "MockSkyboxAiHttpClient.generated.h"

UCLASS()
class SKYBOXAIAPI_API UMockSkyboxAiHttpClient : public USKyboxAiHttpClient
{
  GENERATED_BODY()

public:
  UMockSkyboxAiHttpClient();

  virtual void MakeAPIRequest(const FString& Endpoint, const FSkyboxAiHttpHeaders& Headers, const FString& Body, FSkyboxAiHttpCallback Callback) override;

private:
  FString MockOutPath;

  FString LoadMockResponse(const FString& Filename) const;
};
