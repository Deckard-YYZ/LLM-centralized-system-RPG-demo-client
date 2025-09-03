// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatableActorCppBase.h"

// Sets default values
AChatableActorCppBase::AChatableActorCppBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChatableActorCppBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChatableActorCppBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

