// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpaceShooterPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SpaceShooterPlayerController.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
//#include "GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Editor/EditorEngine.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SpaceShooterProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/Components/LineBatchComponent.h"

ASpaceShooterPawn::ASpaceShooterPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = true; // Don't rotate camera with controller
	bFindCameraComponentWhenViewTarget = true;

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	MaxSpeed = 4000.f;
	MinSpeed = 500.f;
	CurrentForwardSpeed = 500.f;
}

void ASpaceShooterPawn::Tick(float DeltaSeconds)
{
	//const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	//AddActorLocalOffset(LocalMove, true);
	//PlaneMesh->AddForce(LocalMove);
	FVector LocalMove = PlaneMesh->GetForwardVector() * (CurrentForwardSpeed * DeltaSeconds);
	PlaneMesh->SetPhysicsLinearVelocity(LocalMove, true);

	//// Calculate change in rotation this frame
	//FRotator DeltaRotation(0,0,0);
	//DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	//DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	//DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	//AddActorLocalRotation(DeltaRotation);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ASpaceShooterPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	/*FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));*/
}


void ASpaceShooterPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &ASpaceShooterPawn::ThrustInput);
	PlayerInputComponent->BindAxis("MoveUp", this, &ASpaceShooterPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASpaceShooterPawn::MoveRightInput);
	PlayerInputComponent->BindAxis("Roll", this, &ASpaceShooterPawn::Roll);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction("FireProjectile", IE_Pressed, this, &ASpaceShooterPawn::FireProjectile);
}

void ASpaceShooterPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ASpaceShooterPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	//float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	//// When steering, we decrease pitch slightly
	//TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	//// Smoothly interpolate to target pitch speed
	//CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	//FVector upVector = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::Z);
	AddActorLocalRotation(FRotator(Val, 0, 0));
	//PlaneMesh->SetPhysicsAngularVelocity(upVector * Val, true);
}

void ASpaceShooterPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	//float TargetYawSpeed = (Val * TurnSpeed);

	//// Smoothly interpolate to target yaw speed
	//CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	//// Is there any left/right input?
	//const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	//// If turning, yaw value is used to influence roll
	//// If not turning, roll to reverse current roll value.
	//float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	//// Smoothly interpolate roll speed
	//CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
	//FVector rightVector = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::Y);
	AddActorLocalRotation(FRotator(0, Val, 0));
	//PlaneMesh->SetPhysicsAngularVelocity(rightVector * Val, true);
}

void ASpaceShooterPawn::Roll(float Val)
{
	// Target yaw speed is based on input
	//float TargetYawSpeed = (Val * TurnSpeed);

	//// Smoothly interpolate to target yaw speed
	//CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	//// Is there any left/right input?
	//const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	//// If turning, yaw value is used to influence roll
	//// If not turning, roll to reverse current roll value.
	//float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	//// Smoothly interpolate roll speed
	//CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	AddActorLocalRotation(FRotator(0, 0, Val));
	//PlaneMesh->SetPhysicsAngularVelocity(GetActorRotation().Vector() * Val, true);
}

void ASpaceShooterPawn::FireProjectile()
{
	if (ProjectileTemplate != nullptr)
	{
		ASpaceShooterPlayerController *pc = Cast<ASpaceShooterPlayerController>(this->Controller);
		if (pc)
		{
			
			//FVector ShootDir = pc->PlayerCameraManager->GetCameraLocation();
			////ShootDir = FRotationMatrix(RootComponent->GetComponentToWorld().Rotator()).GetScaledAxis(EAxis::X);
			////FVector Origin = GetMuzzleLocation();

			//AActor *ownerOwner = GetOwner();
			////ACharacter *Character = Cast<ACharacter>(ownerOwner);
			////FRotator rotation = ShootDir.GetSafeNormal().Rotation();
			//FRotator rotation = pc->PlayerCameraManager->GetCameraRotation().GetNormalized();
			FRotator ShootFromRotation;
			FVector ShootFromLocation;
			GetShootFromPoint(ShootFromLocation, ShootFromRotation);
			//pc->GetActorEyesViewPoint(ShootDir, rotation);
			/*if (Character)
			{
				rotation = Character->GetViewRotation().GetNormalized();
			}*/
			//rotation = this->GetViewRotation().GetNormalized();
			FTransform SpawnTM(ShootFromRotation, ShootFromLocation);
			ASpaceShooterProjectile *projectile = Cast<ASpaceShooterProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileTemplate, SpawnTM));
			if (projectile)
			{
				projectile->Instigator = this;
				projectile->SetOwner(this);
				this->MoveIgnoreActorAdd(projectile);
				//Projectile->SetActorRotation(Rotation);
				//Projectile->InitVelocity(ShootDir);
				UGameplayStatics::FinishSpawningActor(projectile, SpawnTM);
			}
		}
	}
}

void ASpaceShooterPawn::GetShootFromPoint(FVector& Location, FRotator& Rotation) const
{
	Location = FVector::ZeroVector;
	Rotation = FRotator::ZeroRotator;
	ASpaceShooterPlayerController *pc = Cast<ASpaceShooterPlayerController>(this->Controller);
	if (pc)
	{
		FRotator rotation = pc->PlayerCameraManager->GetCameraRotation().GetNormalized();
		FVector StartTrace = pc->PlayerCameraManager->GetCameraLocation();
		FVector MaxDistance = StartTrace + (rotation.Vector() * 100000); //1000 units away
		AActor *ownerOwner = GetOwner();
		FHitResult OtherImpact;
		FVector OtherTraceStart = MaxDistance;
		UWorld *World = GetWorld();
		//ULineBatchComponent* const LineBatcher = World->LineBatcher;
		//Trace off into the distance, wherever the player is looking at
		if (World->LineTraceSingleByChannel(OtherImpact, StartTrace, MaxDistance, ECollisionChannel::ECC_MAX, FCollisionQueryParams(TEXT("Camera"), false, this)))
		{
			OtherTraceStart = OtherImpact.ImpactPoint;
		}

		/*if (LineBatcher != nullptr)
		{
			LineBatcher->DrawLine(StartTrace, OtherTraceStart, FLinearColor::Red, 1, 5.0f, 0.1f);
		}*/

		TArray<FHitResult> results;
		FCollisionObjectQueryParams objectQueryParams;
		objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		GetWorld()->LineTraceMultiByObjectType(results, OtherTraceStart, this->GetActorLocation(), objectQueryParams, FCollisionQueryParams(TEXT("Camera"), false));
		/*if (LineBatcher != nullptr)
		{
			LineBatcher->DrawLine(OtherTraceStart, this->GetActorLocation(), FLinearColor::Green, 1, 5.0f, 0.1f);
		}*/
		for (auto impact : results)
		{
			if (impact.Actor != nullptr && impact.Actor == this)
			{
				Rotation = FRotationMatrix::MakeFromX(OtherTraceStart - impact.ImpactPoint).Rotator().GetNormalized();
				Location = impact.ImpactPoint + (Rotation.Vector() * 1000);
				return;
			}
		}

		/*if (LineBatcher != nullptr)
		{
			LineBatcher->DrawLine(StartTrace, Location, FLinearColor::Blue, 1, 5.0f, 0.1f);
		}*/
	}
}

//static ULineBatchComponent* GetDebugLineBatcher(const UWorld* InWorld, bool bPersistentLines, float LifeTime, bool bDepthIsForeground)
//{
//	return (InWorld ? (bDepthIsForeground ? InWorld->ForegroundLineBatcher : ((bPersistentLines || (LifeTime > 0.f)) ? InWorld->PersistentLineBatcher : InWorld->LineBatcher)) : NULL);
//}
