// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceShooterProjectile.h"
#include "SpaceShooter.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ASpaceShooterProjectile::ASpaceShooterProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetSimulatePhysics(false);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	/*CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");*/
	//CollisionComp->OnComponentHit.AddDynamic(this, &ANimModProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	ParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = true;
	ParticleComp->bAutoDestroy = true;
	ParticleComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;

}

void ASpaceShooterProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MovementComp->OnProjectileStop.AddDynamic(this, &ASpaceShooterProjectile::OnImpact);
	MovementComp->InitialSpeed = MaxSpeed;
	MovementComp->MaxSpeed = MaxSpeed;
	MovementComp->bShouldBounce = false;
	MovementComp->Velocity *= MaxSpeed;

	CollisionComp->MoveIgnoreActors.Add(Instigator);
	AActor *owner = GetOwner();
	if(owner != nullptr)
		CollisionComp->MoveIgnoreActors.Add(owner);

	/*RadialForceComp->ForceStrength = RadialForce;
	RadialForceComp->ImpulseStrength = RadialForce;

	ANimModWeapon_Projectile* OwnerWeapon = Cast<ANimModWeapon_Projectile>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}*/

	//SetLifeSpan(WeaponConfig.ProjectileLife);
	MyController = GetInstigatorController();
}

void ASpaceShooterProjectile::InitVelocity(FVector& ShootDirection)
{
	if (MovementComp)
	{
		MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
	}
}

void ASpaceShooterProjectile::OnImpact(const FHitResult& HitResult)
{
	if (Role == ROLE_Authority && !bExploded)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

void ASpaceShooterProjectile::Explode(const FHitResult& Impact)
{
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	// effects and damage origin shouldn't be placed inside mesh at impact point
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	/*if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamageWithFalloff
		(
			this,
			WeaponConfig.ExplosionDamage,
			5.0f,
			NudgedImpactLocation,
			0.0f,
			WeaponConfig.ExplosionRadius,
			1.0f,
			WeaponConfig.DamageType,
			TArray<AActor*>(),
			this,
			MyController.Get()
		);
	}*/

	/*if (ExplosionTemplate)
	{
		const FRotator SpawnRotation = Impact.ImpactNormal.Rotation();

		ANimModExplosionEffect* EffectActor = GetWorld()->SpawnActorDeferred<ANimModExplosionEffect>(ExplosionTemplate, NudgedImpactLocation, SpawnRotation);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(SpawnRotation, NudgedImpactLocation));
		}
	}*/

	/*if (RadialForceComp)
	{
		RadialForceComp->SetWorldLocation(NudgedImpactLocation);
		RadialForceComp->FireImpulse();
	}*/
	OnExploded(NudgedImpactLocation, Impact.GetActor());

	bExploded = true;
}

void ASpaceShooterProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	// give clients some time to show explosion
	SetLifeSpan(0.5f);
}

///CODE_SNIPPET_START: AActor::GetActorLocation AActor::GetActorRotation
void ASpaceShooterProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorRotation().Vector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(TEXT("ProjClient"), true, Instigator)))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}
///CODE_SNIPPET_END

void ASpaceShooterProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void ASpaceShooterProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpaceShooterProjectile, bExploded);
}

