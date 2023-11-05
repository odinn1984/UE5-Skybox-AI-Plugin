#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

typedef TSharedPtr<SListView<TSharedPtr<FText>>> FSkyboxAIWidgetListView;

class USkyboxApi;
struct FSkyboxAIWidgetData;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAIWidget, Log, All);

DECLARE_DELEGATE_OneParam(FOnSkyboxAIWidgetDataChanged, const FSkyboxAIWidgetData &);

struct FSkyboxAIWidgetData
{
  bool bEnrichPrompt = false;

  FText Prompt;
  FText NegativeText;
  FText Category;
  FText ExportType;
};

class SSkyboxAIWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SSkyboxAIWidget)
      : _bEnrichPrompt(false),
        _Prompt(FText::FromString(TEXT(""))),
        _NegativeText(FText::FromString(TEXT(""))),
        _Category(FText::FromString(TEXT(""))),
        _ExportType(FText::FromString(TEXT(""))),
        _OnSkyboxAIWidgetDataChanged()
    {
    }

    SLATE_ARGUMENT(bool, bEnrichPrompt)
    SLATE_ARGUMENT(FText, Prompt)
    SLATE_ARGUMENT(FText, NegativeText)
    SLATE_ARGUMENT(FText, Category)
    SLATE_ARGUMENT(FText, ExportType)
    SLATE_EVENT(FOnSkyboxAIWidgetDataChanged, OnSkyboxAIWidgetDataChanged)
  SLATE_END_ARGS()

  void Construct(const FArguments &InArgs);

private:
  FOnSkyboxAIWidgetDataChanged OnSkyboxAIWidgetDataChanged;

  FSkyboxAIWidgetData WidgetData;

  TSharedPtr<USkyboxApi> SkyboxAPI;

  FSkyboxAIWidgetListView CategoryListView;
  FSkyboxAIWidgetListView ExportTypeListView;

  TArray<TSharedPtr<FText>> Categories;
  TArray<TSharedPtr<FText>> FilteredCategories = Categories;

  TArray<TSharedPtr<FText>> ExportTypes;
  TArray<TSharedPtr<FText>> FilteredExportTypes = ExportTypes;

  void LoadCategoriesFromList(const TMap<int, FString> &List);
  void LoadExportTypesFromList(const TMap<int, FString> &List);
  void LoadViewListFromMap(
    TArray<TSharedPtr<FText>> &OutValues,
    TArray<TSharedPtr<FText>> &OutFilteredValues,
    const FSkyboxAIWidgetListView &OutListView,
    const TMap<int, FString> &InList,
    FText &CurrentValue,
    const FString &InListSource) const;
  void UpdateListViewSelection(
    const FSkyboxAIWidgetListView &ListView,
    TArray<TSharedPtr<FText>> &List,
    FText &CurrentValue) const;

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

  static bool ListContains(TArray<TSharedPtr<FText>> &List, const FText &Value);
  static int FindInList(TArray<TSharedPtr<FText>> &List, const FText &Value);
};
