#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

typedef TSharedPtr<SListView<TSharedPtr<FText>>> FSkyboxAIWidgetListView;
typedef std::tuple<int, FString> FSkyboxAIWidgetTuple;

#define TUPLE_KEY_IDX 0
#define TUPLE_VALUE_IDX 1

class USkyboxApi;
struct FSkyboxAIWidgetData;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAIWidget, Log, All);

DECLARE_DELEGATE_OneParam(FOnSkyboxAIWidgetDataChanged, const FSkyboxAIWidgetData &);

struct FSkyboxAIWidgetData
{
  bool bEnrichPrompt = false;

  FText Prompt;
  FText NegativeText;
  FSkyboxAIWidgetTuple Category;
  FSkyboxAIWidgetTuple ExportType;
};

class SSkyboxAIWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SSkyboxAIWidget)
      : _bEnrichPrompt(false),
        _Prompt(FText::FromString(TEXT(""))),
        _NegativeText(FText::FromString(TEXT(""))),
        _Category(std::make_tuple(0, TEXT(""))),
        _ExportType(std::make_tuple(0, TEXT(""))),
        _OnSkyboxAIWidgetDataChanged()
    {
    }

    SLATE_ARGUMENT(bool, bEnrichPrompt)
    SLATE_ARGUMENT(FText, Prompt)
    SLATE_ARGUMENT(FText, NegativeText)
    SLATE_ARGUMENT(FSkyboxAIWidgetTuple, Category)
    SLATE_ARGUMENT(FSkyboxAIWidgetTuple, ExportType)
    SLATE_EVENT(FOnSkyboxAIWidgetDataChanged, OnSkyboxAIWidgetDataChanged)
  SLATE_END_ARGS()

  void Construct(const FArguments &InArgs);

private:
  FOnSkyboxAIWidgetDataChanged OnSkyboxAIWidgetDataChanged;

  FSkyboxAIWidgetData WidgetData;

  TSharedPtr<USkyboxApi> SkyboxAPI;

  FSkyboxAIWidgetListView CategoryListView;
  FSkyboxAIWidgetListView ExportTypeListView;

  TMap<int, FString> Categories;
  TArray<TSharedPtr<FText>> FilteredCategories;

  TMap<int, FString> ExportTypes;
  TArray<TSharedPtr<FText>> FilteredExportTypes;

  void LoadCategoriesFromList(const TMap<int, FString> &List);
  void LoadExportTypesFromList(const TMap<int, FString> &List);
  void LoadViewListFromMap(
    TMap<int, FString> &OutValues,
    TArray<TSharedPtr<FText>> &OutFilteredValues,
    const FSkyboxAIWidgetListView &OutListView,
    const TMap<int, FString> &InList,
    FSkyboxAIWidgetTuple &CurrentValue,
    const FString &InListSource) const;
  void UpdateListViewSelection(
    const FSkyboxAIWidgetListView &ListView,
    TMap<int, FString> &Map,
    TArray<TSharedPtr<FText>> &List,
    FSkyboxAIWidgetTuple &CurrentValue) const;

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

  static TSharedPtr<SNotificationItem> ShowSuccessMessage(
    const FText &Message,
    const TSharedPtr<SNotificationItem> &InNotificationItem = nullptr);
  static TSharedPtr<SNotificationItem> ShowPendingMessage(
    const FText &Message,
    const TSharedPtr<SNotificationItem> &InNotificationItem = nullptr);
  static TSharedPtr<SNotificationItem> ShowFailedMessage(
    const FText &Message,
    const TSharedPtr<SNotificationItem> &InNotificationItem = nullptr);
  static TSharedPtr<SNotificationItem> ShowMessage(
    const FText &Message,
    const SNotificationItem::ECompletionState State,
    const TSharedPtr<SNotificationItem> &InNotificationItem = nullptr);

  void InitSkyboxAiApi();
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

template <typename ListType, typename FindValue> bool SSkyboxAIWidget::ListContains(
  TArray<ListType> &List,
  const FindValue &Value,
  TFunction<bool(const ListType &, const FindValue &)> Equals) const
{
  return FindInList<ListType, FindValue>(List, Value, Equals) != INDEX_NONE;
}

template <typename ListType, typename FindValue> int SSkyboxAIWidget::FindInList(
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
