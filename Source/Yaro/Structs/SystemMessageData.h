#pragma once
#include "Engine/DataTable.h"
#include "SystemMessageData.generated.h"


USTRUCT(BlueprintType)
struct FSystemMessage : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (MultiLine = true))
	FText MessageText;

};
