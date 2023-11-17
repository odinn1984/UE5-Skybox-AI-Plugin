#pragma once

#include "CoreMinimal.h"
#include "ImagineProvider.generated.h"

class USKyboxAiHttpClient;

struct FImagineGetExportsResponse;

typedef TFunction<void(FImagineGetExportsResponseRequest*, int StatusCode, bool bConnectedSuccessfully)> FGetRequestsObfuscatedIdCallback;

USTRUCT()
struct FImagineGetExportsResponseRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString obfuscated_id = TEXT("");

	UPROPERTY()
	FString status = TEXT("");

	UPROPERTY()
	FString error_message = TEXT("");

	UPROPERTY()
	FString file_url = TEXT("");

	UPROPERTY()
	FString depth_map_url = TEXT("");

	UPROPERTY()
	FString title = TEXT("");

	UPROPERTY()
	FString prompt = TEXT("");

	UPROPERTY()
	FString negative_text = TEXT("");

	UPROPERTY()
	int skybox_style_id = INDEX_NONE;

	UPROPERTY()
	FString skybox_style_name = TEXT("");
};

USTRUCT()
struct FImagineGetExportsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FImagineGetExportsResponseRequest request = FImagineGetExportsResponseRequest();
};

UCLASS()
class SKYBOXAIAPI_API UImagineProvider : public UObject
{
	GENERATED_BODY()

public:
	explicit UImagineProvider();

	FORCEINLINE USKyboxAiHttpClient* Client() const { return ApiClient; }

	void SetClient(USKyboxAiHttpClient* InAPIClient);

	virtual void GetRequests(const uint32 Id, FGetRequestsObfuscatedIdCallback Callback) const;
	virtual void GetRequestsObfuscatedId(const FString Id, FGetRequestsObfuscatedIdCallback Callback) const;

private:
	UPROPERTY()
	TObjectPtr<USKyboxAiHttpClient> ApiClient;

	virtual bool IsClientValid() const;
};
