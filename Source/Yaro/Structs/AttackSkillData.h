#pragma once

#include "Engine/DataTable.h"
#include "AttackSkillData.generated.h"

USTRUCT(BlueprintType)
struct FAttackSkillData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray <UClass*> Skills;
};