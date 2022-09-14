// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h" // generated.h 위에 써야함

#include "DialogueUI.generated.h"


/**
 * 
 */


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


UCLASS()
class YARO_API UDialogueUI : public UUserWidget
{
	GENERATED_BODY()


protected:
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NPCText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	float DelayBetweenLetters = 0.06f;

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationShowMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationHideMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnShowMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnHideMessageUI();
	

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnResetOptions();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnSetOption(int32 Option, const FText& OptionText);




	void SetMessage(const FString& Text);

	void SetCharacterName(const FString& Text);

	void AnimateMessage(const FString& Text);

	void InitializeDialogue(class UDataTable* DialogueTable);

	UFUNCTION(BlueprintCallable)
	void Interact();
	 


private:
	
	FTimerHandle TimerHandle;

	UFUNCTION()
	void OnTimerCompleted();

	FString InitialMessage;

	FString OutputMessage;

	int32 iLetter;


	TArray<FNPCDialogue*> Dialogue;

	int32 MessageIndex;

	int32 RowIndex;

	UFUNCTION()
	void OnAnimationTimerCompleted();


public:

	int32 CurrentState = 0; // 0 = None, 1 = Animating, 2 = Text Completed, 3 = Dialogue is waiting for replies
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SelectedReply;


	//플레이어 대답 버튼들 Visibility 때문에 어쩔 수 없이 만든 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumOfReply; 

	UPROPERTY(EditAnywhere)
	class AMain* Main;

	UPROPERTY(EditAnywhere)
	class AMainPlayerController* MainPlayerController;
};
