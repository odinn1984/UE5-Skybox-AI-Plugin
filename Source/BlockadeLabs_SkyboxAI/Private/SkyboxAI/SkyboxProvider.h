#pragma once

#include "CoreMinimal.h"
#include "SkyboxProvider.generated.h"

typedef const TMap<int, FString> FSkyboxAiCategories;
typedef const TMap<int, FString> FSkyboxAiExportTypes;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCategoriesRetrieved, FSkyboxAiCategories&);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExportTypesRetrieved, FSkyboxAiExportTypes&);

class USKyboxAiHttpClient;

// ReSharper disable CppUE4CodingStandardNamingViolationWarning

USTRUCT(BlueprintType)
struct FSkyboxStyleFamily
{
  GENERATED_BODY()

  UPROPERTY()
  int id;

  UPROPERTY()
  FString name;
};

USTRUCT(BlueprintType)
struct FSkyboxStyle
{
  GENERATED_BODY()

  UPROPERTY()
  int id;

  UPROPERTY()
  FString name;

  UPROPERTY()
  int max_char;

  UPROPERTY()
  int negative_text_max_char;

  UPROPERTY()
  FString image;

  UPROPERTY()
  int sort_order;

  UPROPERTY()
  int premium;

  UPROPERTY()
  TArray<FSkyboxStyleFamily> skybox_style_families;
};

USTRUCT()
struct FSkyboxExportType
{
  GENERATED_BODY()

  UPROPERTY()
  int id;

  UPROPERTY()
  FString key;

  UPROPERTY()
  FString name;

  UPROPERTY()
  bool premium_feature;
};

// ReSharper restore CppUE4CodingStandardNamingViolationWarning

UCLASS()
class USkyboxProvider : public UObject
{
  GENERATED_BODY()

public:
  FOnCategoriesRetrieved OnCategoriesRetrieved;
  FOnExportTypesRetrieved OnExportTypesRetrieved;

  explicit USkyboxProvider();

  virtual void SetClient(USKyboxAiHttpClient *InAPIClient);
  virtual void Categories() const;
  virtual void ExportTypes() const;

protected:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> APIClient;
};
