// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h" // generated.h 위에 써야함
#include "Yaro/Character/YaroCharacter.h"
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
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UGameManager* GameManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UDialogueManager* DialogueManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UNPCManager* NPCManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AMain* Player;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AMainPlayerController* MainPlayerController;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AYaroCharacter* Momo;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AYaroCharacter* Luko;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AYaroCharacter* Vovo;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AYaroCharacter* Vivi;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AYaroCharacter* Zizi;


	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NPCText;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UTextBlock* CharacterNameText;
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	float DelayBetweenLetters = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* SoundCueMessage;

	UPROPERTY(EditAnywhere)
	class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere)
	class USoundBase* GriffonSound;

	UPROPERTY(EditAnywhere)
	class USoundBase* AfterAllCombat;

	bool bInputDisabled = false;


	int32 CurrentState; // 0 = None, 1 = Animating, 2 = Text Completed, 3 = Dialogue is waiting for replies

	UPROPERTY(BlueprintReadWrite)
	int32 SelectedReply;

	UPROPERTY(BlueprintReadWrite)
	int32 RowIndex;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 MessageIndex;

	//플레이어 대답 버튼들 Visibility 때문에 어쩔 수 없이 만든 변수
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 NumOfReply;

	// 대화 끝나고 바로 대화 또 못하게끔(애니메이션 시간이 필요)
	UPROPERTY(BlueprintReadWrite)
	bool bCanStartDialogue = true;

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

	UFUNCTION(BlueprintImplementableEvent)
	void ActivateSpeechBubble();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnGriffon();

private:
	FString InitialMessage;

	FString OutputMessage;
	
	int32 iLetter;

	TArray<FNPCDialogue*> Dialogue;


	UFUNCTION()
	void OnAnimationTimerCompleted();

	FTimerHandle TextTimer;

	FTimerHandle AutoDialogueTimer;

public:
 
	UFUNCTION(BlueprintCallable)
		int32 GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable)
		int32 GetSelectedReply() const { return SelectedReply; }

	UFUNCTION(BlueprintCallable)
		int32 GetRowIndex() const { return RowIndex; }

	bool CanStartDialogue() const { return bCanStartDialogue; }

    FORCEINLINE void StartAnimatedMessage() { AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString()); }

	void AutoCloseDialogue();
	
	void AutoDialogue();

	void ClearAutoDialogueTimer();

	UFUNCTION(BlueprintCallable)
		bool IsInputDisabled() const { return bInputDisabled; }

	UFUNCTION(BlueprintCallable)
	void SetInputDisabled(bool Value) { bInputDisabled = Value; }
};
