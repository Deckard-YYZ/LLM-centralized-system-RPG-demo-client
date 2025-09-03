// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UCLASS()
class THEPROJECT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Give the AIController a readable UObject name at spawn time. */
	virtual void SpawnDefaultController() override;

	/** Sanitize a string into a valid UObject name base (letters, digits, underscore). */
	static FString SanitizeObjectNameBase(const FString& In);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** A human-readable name you can set in BP/Details. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
	FText DisplayName;

	/**
	 * Sets the editor label shown in the World Outliner / PIE.
	 * Editor-only; does nothing in packaged builds.
	 */
	UFUNCTION(BlueprintCallable, Category="Identity")
	void SetEditorLabel(const FString& NewLabel);

	/**
	 * Renames the underlying UObject (what GetName() returns).
	 * Use carefully; prefer SetEditorLabel for readability.
	 */
	UFUNCTION(BlueprintCallable, Category="Identity")
	void RenameObjectInternal(const FString& NewInternalName);

	/**
	 * Convenience: applies DisplayName to the Actor Label (editor) and
	 * optionally to the internal UObject name.
	 *
	 * @param bSetEditorLabel   If true, set the Editor label from DisplayName (editor-only).
	 * @param bRenameInternal   If true, rename the internal UObject to DisplayName.ToString().
	 */
	UFUNCTION(BlueprintCallable, Category="Identity")
	void ApplyDisplayName(bool bSetEditorLabel = true, bool bRenameInternal = false);

};
