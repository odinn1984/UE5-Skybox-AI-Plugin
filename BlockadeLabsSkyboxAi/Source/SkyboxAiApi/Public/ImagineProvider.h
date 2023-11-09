#pragma once

#include "CoreMinimal.h"
#include "ImagineProvider.generated.h"

class USKyboxAiHttpClient;

struct FImagineGetExportsResponse;

typedef TFunction<void(FImagineGetExportsResponse *, int StatusCode, bool bConnectedSuccessfully)> FGetRequestsCallback;


USTRUCT()
struct FImagineGetExportsResponseRequest
{
  GENERATED_BODY()

  UPROPERTY()
  FString obfuscated_id;

  UPROPERTY()
  FString status;

  UPROPERTY()
  FString error_message;

  UPROPERTY()
  FString file_url;

  UPROPERTY()
  FString title;
};

USTRUCT()
struct FImagineGetExportsResponse
{
  GENERATED_BODY()

  UPROPERTY()
  FImagineGetExportsResponseRequest request;
};

UCLASS()
class SKYBOXAIAPI_API UImagineProvider : public UObject
{
  GENERATED_BODY()

public:
  explicit UImagineProvider();

  void SetClient(USKyboxAiHttpClient *InAPIClient);

  virtual void GetRequests(const FString Id, FGetRequestsCallback Callback) const;

private:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> ApiClient;

  virtual bool IsClientValid() const;
};
