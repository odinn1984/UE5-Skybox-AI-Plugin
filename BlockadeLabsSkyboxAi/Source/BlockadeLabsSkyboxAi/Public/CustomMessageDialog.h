#pragma once

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 2

enum class EAppMsgCategory : uint8
{
  Warning,
  Error,
  Success,
  Info,
};

#endif

struct FCustomMessageDialog : FMessageDialog
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 2

  static EAppReturnType::Type Open(
    EAppMsgCategory MessageCategory,
    EAppMsgType::Type MessageType,
    const FText &Message,
    const FText &Title);

#endif
};
