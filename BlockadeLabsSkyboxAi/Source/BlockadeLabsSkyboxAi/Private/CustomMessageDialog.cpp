#include "CustomMessageDialog.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 1

EAppReturnType::Type FCustomMessageDialog::Open(
    EAppMsgCategory MessageCategory,
    EAppMsgType::Type MessageType,
    const FText &Message,
    const FText &Title)
{
  const FText *TitlePtr = &Title;

  return FMessageDialog::Open(MessageType, Message, TitlePtr);
}

#endif
