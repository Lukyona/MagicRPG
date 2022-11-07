// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h" // generated.h ���� �����

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* SoundCueMessage;

	UPROPERTY(EditAnywhere)
	class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemySpawner> Spawner;


public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationShowMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationHideMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnResetOptions();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnSetOption(int32 Option, const FText& OptionText);




	void SetMessage(const FString& Text);

	void SetCharacterName(const FString& Text);

	void AnimateMessage(const FString& Text);

	UFUNCTION(BlueprintCallable)
	void InitializeDialogue(class UDataTable* DialogueTable);

	UFUNCTION(BlueprintCallable)
    void Interact();
   
	void DialogueEvents();

private:
	
	FString InitialMessage;

	FString OutputMessage;

	int32 iLetter;


	TArray<FNPCDialogue*> Dialogue;



	UFUNCTION()
	void OnAnimationTimerCompleted();


public:
    UPROPERTY(BlueprintReadWrite)
	int32 CurrentState; // 0 = None, 1 = Animating, 2 = Text Completed, 3 = Dialogue is waiting for replies
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SelectedReply;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 RowIndex;

    UPROPERTY(BlueprintReadWrite)
    int32 MessageIndex;

	//�÷��̾� ��� ��ư�� Visibility ������ ��¿ �� ���� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumOfReply; 

	UPROPERTY(EditAnywhere)
	class AMain* Main;

	UPROPERTY(EditAnywhere)
	class AMainPlayerController* MainPlayerController;

	// ��ȭ ������ �ٷ� ��ȭ �� ���ϰԲ�(�ִϸ��̼� �ð��� �ʿ�)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStartDialogue = true;

	FTimerHandle TimerHandle;

    FORCEINLINE void StartAnimatedMessage() { AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString()); }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisableMouseAndKeyboard = false;

	void AutoCloseDialogue();

    FTimerHandle OnceTimer;

	void AllNpcLookAtPlayer();

	void AllNpcDisableLookAt();

	void AllNpcStopFollowPlayer();

	void AutoDialogue();

};
