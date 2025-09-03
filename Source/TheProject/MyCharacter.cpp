// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"

#include "AIController.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITOR
	// Automatically apply the DisplayName when the game starts in editor / PIE.
	if (!DisplayName.IsEmpty())
	{
		// Set editor label to DisplayName.
		SetEditorLabel(DisplayName.ToString());

		// If you also want to rename the UObject internal name (optional), uncomment this:
		RenameObjectInternal(DisplayName.ToString());
	}
#endif
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter::SetEditorLabel(const FString& NewLabel)
{
#if WITH_EDITOR
	// Works in editor/PIE only; safe no-op in packaged builds.
	if (!NewLabel.IsEmpty())
	{
		SetActorLabel(NewLabel);
	}
#endif
}

void AMyCharacter::RenameObjectInternal(const FString& NewInternalName)
{
	if (NewInternalName.IsEmpty())
	{
		return;
	}

	// NOTE:
	// Renaming changes the UObject internal name (GetName()).
	// Avoid if systems rely on the original name. Useful for debugging.
	// This will keep the same outer; flags default to REN_None for runtime safety.
	Rename(*NewInternalName);
}

void AMyCharacter::ApplyDisplayName(bool bSetEditorLabel /*= true*/, bool bRenameInternal /*= false*/)
{
	const FString AsString = DisplayName.ToString();

	if (bSetEditorLabel && !AsString.IsEmpty())
	{
		SetEditorLabel(AsString);
	}

	if (bRenameInternal && !AsString.IsEmpty())
	{
		RenameObjectInternal(AsString);
	}
}

//Below may delete when shipping
FString AMyCharacter::SanitizeObjectNameBase(const FString& In)
{
	FString Out = In;
	Out.TrimStartAndEndInline();

	for (int32 i = 0; i < Out.Len(); ++i)
	{
		TCHAR& C = Out[i];
		if (!FChar::IsAlnum(C) && C != TEXT('_'))
		{
			C = TEXT('_');
		}
	}
	return Out;
}

void AMyCharacter::SpawnDefaultController()
{
	// Only spawn on authority & if we don't have one yet
	if (!HasAuthority() || Controller != nullptr || AIControllerClass == nullptr)
	{
		Super::SpawnDefaultController();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		Super::SpawnDefaultController();
		return;
	}

	// Build a nice base name from DisplayName (fallbacks applied)
	FString BaseName;
	if (!DisplayName.IsEmpty())
	{
		BaseName = DisplayName.ToString();
	}
#if WITH_EDITOR
	if (BaseName.IsEmpty())
	{
		BaseName = GetActorLabel();
	}
#endif
	if (BaseName.IsEmpty())
	{
		BaseName = GetName();
	}
	BaseName = SanitizeObjectNameBase(BaseName);
	if (BaseName.IsEmpty())
	{
		BaseName = TEXT("AIController");
	}

	// Make unique & spawn with that name
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Name = MakeUniqueObjectName(World, AIControllerClass, *BaseName);
	// Params.Owner = this;
	// Params.Instigator = this;

	AAIController* NewController = World->SpawnActor<AAIController>(AIControllerClass, GetActorLocation(), GetActorRotation(), Params);
	if (NewController)
	{
		NewController->Possess(this);
	}
	else
	{
		// Fallback to engine behavior if something failed
		Super::SpawnDefaultController();
	}
}
