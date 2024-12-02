// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Yaro/Structs/DialogueData.h"
#include "Yaro/Character/YaroCharacter.h"
#include "DialogueUI.generated.h"
/**
 * 
 */

class UGameManager;
class UDialogueManager;
class UNPCManager;
class AMain;
class AMainPlayerController;
class AYaroCharacter;
class UTextBlock;
class USoundBase;
class UDataTable;

UCLASS()
class YARO_API UDialogueUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

protected:
	// Managers
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UGameManager* GameManager;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UDialogueManager* DialogueManager;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNPCManager* NPCManager;

	//Player and player controller
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMain* Player;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMainPlayerController* MainPlayerController;

	//NPCs
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AYaroCharacter* Momo;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AYaroCharacter* Luko;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AYaroCharacter* Vovo;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AYaroCharacter* Vivi;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AYaroCharacter* Zizi;


	//Dialogue properties
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NPCText;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UTextBlock* CharacterNameText;

private:
	TArray<FNPCDialogue*> Dialogue;
	FTimerHandle TextTimer, AutoDialogueTimer;

	FString InitialMessage, OutputMessage;
	int32 iLetter;

	float DelayBetweenLetters = 0.06f;

	bool bInputDisabled = false;

protected:
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

	//Sounds
	UPROPERTY(EditAnywhere)
	USoundBase* TextSound;
	UPROPERTY(EditAnywhere)
	USoundBase* ExplosionSound;
	UPROPERTY(EditAnywhere)
	USoundBase* GriffonSound;
	UPROPERTY(EditAnywhere)
	USoundBase* AfterAllCombatBGM;

public: //Getters and setters
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentState() const 
	{
		return CurrentState; 
	}
	UFUNCTION(BlueprintCallable)
	int32 GetSelectedReply() const
	{
		return SelectedReply; 
	}
	UFUNCTION(BlueprintCallable)
	int32 GetRowIndex() const 
	{
		return RowIndex; 
	}

	bool CanStartDialogue() const 
	{
		return bCanStartDialogue; 
	}

	UFUNCTION(BlueprintCallable)
	bool IsInputDisabled() const 
	{
		return bInputDisabled; 
	}
	UFUNCTION(BlueprintCallable)
	void SetInputDisabled(bool Value) 
	{
		bInputDisabled = Value; 
	}


protected: //Core methods
	void SetMessage(const FString& Text);
	void SetCharacterName(const FString& Text);

	void AnimateMessage(const FString& Text);

	UFUNCTION()
	void OnAnimationTimerCompleted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationShowMessageUI();
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnAnimationHideMessageUI();
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnResetOptions();
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
	void OnSetOption(int32 Option, const FText& OptionText);

	void DialogueEvents();

	UFUNCTION(BlueprintImplementableEvent)
	void ActivateSpeechBubble();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnGriffon();

public:
	UFUNCTION(BlueprintCallable)
	void InitializeDialogue(UDataTable* DialogueTable);

	UFUNCTION(BlueprintCallable)
	void Interact();

	void AutoCloseDialogue();
	void ClearAutoDialogueTimer();
};
