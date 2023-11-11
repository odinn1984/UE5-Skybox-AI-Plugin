#pragma once

#include "CoreMinimal.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "SkyboxProvider.h"
#include "Widgets/Notifications/SNotificationList.h"

typedef TSharedPtr<SListView<TSharedPtr<FText>>> FSkyboxAiWidgetListView;
typedef std::tuple<int, FString, int, int> FSkyboxAiStylesTuple;
typedef std::tuple<int, FString> FSkyboxAiExportTypesTuple;

#define DEFAULT_ID 0
#define DEFAULT_MAX_TEXT_LEN 500

#define TUPLE_KEY_IDX 0
#define TUPLE_VALUE_IDX 1
#define TUPLE_PROMPT_MAX_LEN_IDX 2
#define TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX 3

class USkyboxApi;
class SMultiLineEditableTextBox;

struct FSkyboxAiWidgetData;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiWidget, Log, All);

DECLARE_DELEGATE_OneParam(FOnSkyboxAiWidgetDataChanged, const FSkyboxAiWidgetData &);

struct FSkyboxAiWidgetData
{
  bool bEnrichPrompt = false;

  FText Prompt;
  FText NegativeText;
  FSkyboxAiStylesTuple Category;
  FSkyboxAiExportTypesTuple ExportType;
};

class SSkyboxAiWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SSkyboxAiWidget)
      : _bEnrichPrompt(false),
        _Prompt(FText::FromString(TEXT(""))),
        _NegativeText(FText::FromString(TEXT(""))),
        _Category(std::make_tuple(DEFAULT_ID, TEXT(""), DEFAULT_MAX_TEXT_LEN, DEFAULT_MAX_TEXT_LEN)),
        _ExportType(std::make_tuple(DEFAULT_ID, TEXT(""))),
        _OnSkyboxAiWidgetDataChanged()
    {
    }

    SLATE_ARGUMENT(bool, bEnrichPrompt)
    SLATE_ARGUMENT(FText, Prompt)
    SLATE_ARGUMENT(FText, NegativeText)
    SLATE_ARGUMENT(FSkyboxAiStylesTuple, Category)
    SLATE_ARGUMENT(FSkyboxAiExportTypesTuple, ExportType)
    SLATE_EVENT(FOnSkyboxAiWidgetDataChanged, OnSkyboxAiWidgetDataChanged)
  SLATE_END_ARGS()

  inline static const FText RefreshListsNotificationTitle = FText::FromString(TEXT("Refresh Lists"));
  inline static const FText GenerateSkyboxNotificationTitle = FText::FromString(TEXT("Generate Skybox"));

  void Construct(const FArguments &InArgs);

private:
  FOnSkyboxAiWidgetDataChanged OnSkyboxAiWidgetDataChanged;

  FSkyboxAiWidgetData WidgetData;

  TAtomic<bool> bGeneratePolling = false;
  TAtomic<bool> bExportPolling = false;

  TWeakObjectPtr<USkyboxApi> SkyboxApi;
  TWeakObjectPtr<UBlockadeLabsSkyboxAiSettings> PluginSettings;
  TSharedPtr<SNotificationItem> RefreshListsNotification;
  TSharedPtr<SNotificationItem> GenerateSkyboxNotification;

  TSharedPtr<SButton> GenerateButton;
  TSharedPtr<SButton> RefreshListsButton;
  TSharedPtr<SCheckBox> EnhancePromptCheckbox;
  TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
  TSharedPtr<SMultiLineEditableTextBox> NegativeTextTextBox;
  TSharedPtr<STextBlock> PromptLabel;
  TSharedPtr<STextBlock> NegativeTextLabel;
  FSkyboxAiWidgetListView CategoryListView;
  FSkyboxAiWidgetListView ExportTypeListView;

  TMap<int, FSkyboxListEntry> Categories;
  TArray<TSharedPtr<FText>> FilteredCategories;

  TMap<int, FString> ExportTypes;
  TArray<TSharedPtr<FText>> FilteredExportTypes;

  void LoadCategoriesFromList(const FSkyboxAiStyles &List);
  void LoadExportTypesFromList(const FSkyboxAiExportTypes &List);
  template <typename EntryType, typename TupleType> void LoadViewListFromMap(
    TMap<int, EntryType> &OutValues,
    TArray<TSharedPtr<FText>> &OutFilteredValues,
    const FSkyboxAiWidgetListView &OutListView,
    const TMap<int, EntryType> &InList,
    TupleType &CurrentValue,
    const FString &InListSource,
    TFunction<FText(const EntryType &)> GetText);
  template <typename EntryType, typename TupleType> void UpdateListViewSelection(
    const FSkyboxAiWidgetListView &ListView,
    TMap<int, EntryType> &Map,
    TArray<TSharedPtr<FText>> &List,
    TupleType &CurrentValue,
    TFunction<FText(const EntryType &)> GetText);
  template <typename EntryType, typename TupleType> int SetMapNthValueToCurrentValue(
    TMap<int, EntryType> &Map,
    const int Index,
    TupleType &CurrentValue);

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
  bool ValidateGenerateData() const;
  void StartPollingGenerationStatus(const FString &SkyboxId);
  void PollGenerationStatus(const FString &SkyboxId);
  void StartPollingExportStatus(const FString &SkyboxId);
  void PollExportStatus(const FString &SkyboxId);
  FReply OnRefreshLists();
  void ExecuteRefreshListAsync();

  void UpdateTextCharacterCount(
    const FString &LabelTitle,
    const TSharedPtr<STextBlock> &InPromptLabel,
    const TSharedPtr<SMultiLineEditableTextBox> &InPromptTextBox,
    const int TupleLenIdx) const;

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

template <typename EntryType, typename TupleType> void SSkyboxAiWidget::LoadViewListFromMap(
  TMap<int, EntryType> &OutValues,
  TArray<TSharedPtr<FText>> &OutFilteredValues,
  const FSkyboxAiWidgetListView &OutListView,
  const TMap<int, EntryType> &InList,
  TupleType &CurrentValue,
  const FString &InListSource,
  TFunction<FText(const EntryType &)> GetText)
{
  for (auto &Item : InList)
  {
    OutValues.Add(Item.Key, Item.Value);
    OutFilteredValues.Add(MakeShared<FText>(GetText(Item.Value)));
  }

  if (!OutListView.IsValid())
  {
    return ShowMessage(
      RefreshListsNotification,
      RefreshListsNotificationTitle,
      FText::FromString(FString::Printf(TEXT("[%s] Invalid List View Provided"), *InListSource)),
      SNotificationItem::CS_Fail
      );
  }

  UpdateListViewSelection<EntryType, TupleType>(
    OutListView,
    OutValues,
    OutFilteredValues,
    CurrentValue,
    GetText
    );
}

template <typename EntryType, typename TupleType> void SSkyboxAiWidget::UpdateListViewSelection(
  const FSkyboxAiWidgetListView &ListView,
  TMap<int, EntryType> &Map,
  TArray<TSharedPtr<FText>> &List,
  TupleType &CurrentValue,
  TFunction<FText(const EntryType &)> GetText)
{
  const FString Value = std::get<TUPLE_VALUE_IDX>(CurrentValue);
  const TFunction<bool(const TSharedPtr<FText> &, const FString &)> Equals = [](
    const TSharedPtr<FText> &Item,
    const FString &Value)
  {
    return Item->EqualTo(FText::FromString(Value));
  };

  int MapId = INDEX_NONE;

  if (Value.IsEmpty() || (List.Num() > 0 && ListContains<TSharedPtr<FText>, FString>(List, Value, Equals)))
  {
    if (const int ItemIndex = FindInList<TSharedPtr<FText>, FString>(List, Value, Equals); ItemIndex != INDEX_NONE)
    {
      MapId = SetMapNthValueToCurrentValue<EntryType, TupleType>(Map, ItemIndex, CurrentValue);
    }
  }

  if (List.Num() > 0 && MapId == INDEX_NONE)
  {
    MapId = SetMapNthValueToCurrentValue<EntryType, TupleType>(Map, 0, CurrentValue);
  }

  if (MapId != INDEX_NONE && ListView.IsValid())
  {
    const int ItemIndex = FindInList<TSharedPtr<FText>, FString>(List, GetText(Map[MapId]).ToString(), Equals);
    ListView->SetSelection(List[ItemIndex]);
  }

  NotifyWidgetDataUpdated();
  if (ListView.IsValid()) ListView->RequestListRefresh();
}

template <typename EntryType, typename TupleType> int SSkyboxAiWidget::SetMapNthValueToCurrentValue(
  TMap<int, EntryType> &Map,
  const int Index,
  TupleType &CurrentValue)
{
  int CurrentIndex = 0;
  for (auto It = Map.CreateIterator(); It; ++It)
  {
    if (CurrentIndex++ != Index) continue;

    if constexpr (std::is_same_v<EntryType, FString>)
    {
      CurrentValue = std::make_tuple(It->Key, It->Value);
    }
    else if constexpr (std::is_same_v<EntryType, FSkyboxListEntry>)
    {
      CurrentValue = std::make_tuple(It->Key, It->Value.Name, It->Value.PromptMaxLen, It->Value.NegativeTextMaxLen);
    }
    else
    {
      UE_LOG(SkyboxAiWidget, Error, TEXT("Failed setting default value for [%s]"), *typeid(CurrentValue).name());
    }

    return It->Key;
  }

  return INDEX_NONE;
}

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
