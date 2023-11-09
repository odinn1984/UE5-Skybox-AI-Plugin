#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

typedef TSharedPtr<SListView<TSharedPtr<FText>>> FSkyboxAiWidgetListView;
typedef std::tuple<int, FString> FSkyboxAiWidgetTuple;

#define TUPLE_KEY_IDX 0
#define TUPLE_VALUE_IDX 1

class USkyboxApi;
struct FSkyboxAiWidgetData;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiWidget, Log, All);

DECLARE_DELEGATE_OneParam(FOnSkyboxAiWidgetDataChanged, const FSkyboxAiWidgetData &);

struct FSkyboxAiWidgetData
{
  bool bEnrichPrompt = false;

  FText Prompt;
  FText NegativeText;
  FSkyboxAiWidgetTuple Category;
  FSkyboxAiWidgetTuple ExportType;
};

class SSkyboxAiWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SSkyboxAiWidget)
      : _bEnrichPrompt(false),
        _Prompt(FText::FromString(TEXT(""))),
        _NegativeText(FText::FromString(TEXT(""))),
        _Category(std::make_tuple(0, TEXT(""))),
        _ExportType(std::make_tuple(0, TEXT(""))),
        _OnSkyboxAiWidgetDataChanged()
    {
    }

    SLATE_ARGUMENT(bool, bEnrichPrompt)
    SLATE_ARGUMENT(FText, Prompt)
    SLATE_ARGUMENT(FText, NegativeText)
    SLATE_ARGUMENT(FSkyboxAiWidgetTuple, Category)
    SLATE_ARGUMENT(FSkyboxAiWidgetTuple, ExportType)
    SLATE_EVENT(FOnSkyboxAiWidgetDataChanged, OnSkyboxAiWidgetDataChanged)
  SLATE_END_ARGS()

  inline static const FText RefreshListsNotificationTitle = FText::FromString(TEXT("Refresh Lists"));
  inline static const FText GenerateSkyboxNotificationTitle = FText::FromString(TEXT("Generate Skybox"));

  void Construct(const FArguments &InArgs);

private:
  FOnSkyboxAiWidgetDataChanged OnSkyboxAiWidgetDataChanged;

  FSkyboxAiWidgetData WidgetData;

  TWeakObjectPtr<USkyboxApi> SkyboxApi;
  TWeakObjectPtr<UBlockadeLabsSkyboxAiSettings> PluginSettings;
  TSharedPtr<SNotificationItem> RefreshListsNotification;
  TSharedPtr<SNotificationItem> GenerateSkyboxNotification;

  TSharedPtr<SButton> GenerateButton;
  TSharedPtr<SButton> RefreshListsButton;
  TSharedPtr<SCheckBox> EnhancePromptCheckbox;
  FSkyboxAiWidgetListView CategoryListView;
  FSkyboxAiWidgetListView ExportTypeListView;

  TMap<int, FString> Categories;
  TArray<TSharedPtr<FText>> FilteredCategories;

  TMap<int, FString> ExportTypes;
  TArray<TSharedPtr<FText>> FilteredExportTypes;

  void LoadCategoriesFromList(const TMap<int, FString> &List);
  void LoadExportTypesFromList(const TMap<int, FString> &List);
  void LoadViewListFromMap(
    TMap<int, FString> &OutValues,
    TArray<TSharedPtr<FText>> &OutFilteredValues,
    const FSkyboxAiWidgetListView &OutListView,
    const TMap<int, FString> &InList,
    FSkyboxAiWidgetTuple &CurrentValue,
    const FString &InListSource);
  void UpdateListViewSelection(
    const FSkyboxAiWidgetListView &ListView,
    TMap<int, FString> &Map,
    TArray<TSharedPtr<FText>> &List,
    FSkyboxAiWidgetTuple &CurrentValue) const;

  void AddPrompt(TSharedPtr<SVerticalBox> RootWidget);
  void OnPromptTextChanged(const FText &NewText);
  void OnPromTextCommitted(const FText &NewText, ETextCommit::Type CommitType);

  void AddExportTypeSelector(TSharedPtr<SHorizontalBox> RootWidget);
  void OnExportTypeSearchTextChanged(const FText &NewText);
  TSharedRef<ITableRow> OnExportTypeGenerateRow(TSharedPtr<FText> Item, const TSharedRef<STableViewBase> &OwnerTable);
  void OnExportTypeSelected(TSharedPtr<FText> InItem, ESelectInfo::Type SelectInfo);

  void AddNegativeTextAndCategories(TSharedPtr<SVerticalBox> RootWidget);
  void OnNegativeTextChanged(const FText &NewText);
  void OnNegativeTextCommitted(const FText &NewText, ETextCommit::Type CommitType);

  void AddCategorySelector(TSharedPtr<SHorizontalBox> RootWidget);
  void OnCategorySearchTextChanged(const FText &NewText);
  TSharedRef<ITableRow> OnCategoryGenerateRow(TSharedPtr<FText> Item, const TSharedRef<STableViewBase> &OwnerTable);
  void OnCategorySelected(TSharedPtr<FText> InItem, ESelectInfo::Type SelectInfo);

  void AddBottomRow(TSharedPtr<SVerticalBox> RootWidget);
  void OnEnrichPromptChanged(ECheckBoxState NewState);

  FReply OnGenerateClicked();
  FReply OnRefreshLists();
  void ExecuteRefreshListAsync();

  void ShowMessage(
    TSharedPtr<SNotificationItem> &Notification,
    const FText &Title,
    const FText &Message,
    const SNotificationItem::ECompletionState State);

  void NotifyWidgetDataUpdated() const;

  template <typename ListType, typename FindValue> bool ListContains(
    TArray<ListType> &List,
    const FindValue &Value,
    TFunction<bool(const ListType &, const FindValue &)> Equals) const;

  template <typename ListType, typename FindValue> int FindInList(
    TArray<ListType> &List,
    const FindValue &Value,
    TFunction<bool(const ListType &, const FindValue &)> Equals) const;
};

template <typename ListType, typename FindValue> bool SSkyboxAiWidget::ListContains(
  TArray<ListType> &List,
  const FindValue &Value,
  TFunction<bool(const ListType &, const FindValue &)> Equals) const
{
  return FindInList<ListType, FindValue>(List, Value, Equals) != INDEX_NONE;
}

template <typename ListType, typename FindValue> int SSkyboxAiWidget::FindInList(
  TArray<ListType> &List,
  const FindValue &Value,
  TFunction<bool(const ListType &, const FindValue &)> Equals) const
{
  for (int i = 0; i < List.Num(); i++)
  {
    if (Equals(List[i], Value)) return i;
  }

  return INDEX_NONE;
}
