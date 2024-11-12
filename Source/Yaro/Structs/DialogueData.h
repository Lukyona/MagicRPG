#pragma once
#include "Engine/DataTable.h"
#include "DialogueData.generated.h"


USTRUCT(BlueprintType)
struct FPlayerReplies
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FText ReplyText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 AnswerIndex;

};

USTRUCT(BlueprintType)
struct FNPCDialogue : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FText> Messages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FPlayerReplies> PlayerReplies;
};