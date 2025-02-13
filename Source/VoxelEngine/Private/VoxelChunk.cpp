// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include <array>
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/MeshAttributeUtil.h"

// Sets default values for this component's properties
UVoxelChunk::UVoxelChunk()
{
	PrimaryComponentTick.bCanEverTick = true;

	// DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMesh"));
	// DynamicMeshComponent->SetupAttachment(this);
}

// Called when the game starts
void UVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	FIntVector MinVoxel;
	FIntVector MaxVoxel;
	GetVoxelBoundingBox(MinVoxel, MaxVoxel);
	FVector Location = FVector(MinVoxel.X, MinVoxel.Y, MinVoxel.Z) * VoxelWorld->GetVoxelSizeWorld();
	SetRelativeLocation(Location);	

	DynamicMeshComponent = NewObject<UDynamicMeshComponent>(this);
	check(DynamicMeshComponent);
	DynamicMeshComponent->RegisterComponent();
	FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, false);
	DynamicMeshComponent->AttachToComponent(this, Rules);

	GenerateMesh();
}

void UVoxelChunk::GenerateMesh()
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	checkSlow(DynamicMeshComponent);
	UDynamicMesh* MeshObj = DynamicMeshComponent->GetDynamicMesh();
	checkSlow(MeshObj);
	FDynamicMesh3& Mesh = MeshObj->GetMeshRef();
	Mesh.Clear();
	Mesh.EnableVertexUVs(FVector2f(0, 0));
	Mesh.EnableAttributes();

	ProcessVoxels();

	// Mesh.ReverseOrientation();

	UE::Geometry::FDynamicMesh3::FValidityOptions ValidityOptions;
	UE::Geometry::EValidityCheckFailMode ValidityCheckFailMode = UE::Geometry::EValidityCheckFailMode::Ensure;
	bool bIsValid = Mesh.CheckValidity(ValidityOptions, ValidityCheckFailMode);
	if (bIsValid)
	{
		UE_LOG(LogTemp, Display, TEXT("Dynamic Mesh passed validity check"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic Mesh failed validity check"));
	}
	UE::Geometry::CopyVertexUVsToOverlay(Mesh, *Mesh.Attributes()->PrimaryUV());
	DynamicMeshComponent->SetMaterial(0, VoxelWorld->GetVoxelChunkMaterial());
	DynamicMeshComponent->NotifyMeshUpdated();
	DynamicMeshComponent->SetEnableFlatShading(true);
}

void UVoxelChunk::ProcessVoxels()
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 WorldHeightVoxel = VoxelWorld->GetWorldHeight();

	for (int X = 0; X < ChunkSide; X++)
	{
		for (int Y = 0; Y < ChunkSide; Y++)
		{
			for (int Z = 0; Z < WorldHeightVoxel; Z++)
			{
				ProcessVoxel(X, Y, Z);
			}
		}
	}
}

void UVoxelChunk::ProcessVoxel(int32 X, int32 Y, int32 Z)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	FIntVector ChunkMin;
	FIntVector ChunkMax;
	GetVoxelBoundingBox(ChunkMin, ChunkMax);

	FIntVector CoordTranslated = ChunkMin + FIntVector(X, Y, Z);
	checkSlow(CoordTranslated.X <= ChunkMax.X);
	checkSlow(CoordTranslated.Y <= ChunkMax.Y);
	checkSlow(CoordTranslated.Z <= ChunkMax.Z);

	if (VoxelWorld->IsVoxelTransparent(CoordTranslated))
	{
		return;
	}

	const Voxel& Voxel = VoxelWorld->GetVoxel(X, Y, Z);

	TArray<bool, TFixedAllocator<6>> FacesVisibility;
	FacesVisibility.SetNumUninitialized(6);
	FacesVisibility[0] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y + 1, CoordTranslated.Z); // Right
	FacesVisibility[1] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y - 1, CoordTranslated.Z); // Left
	FacesVisibility[2] = IsFaceVisible(CoordTranslated.X - 1, CoordTranslated.Y, CoordTranslated.Z); // Back
	FacesVisibility[3] = IsFaceVisible(CoordTranslated.X + 1, CoordTranslated.Y, CoordTranslated.Z); // Front
	FacesVisibility[4] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y, CoordTranslated.Z + 1); // Top
	FacesVisibility[5] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y, CoordTranslated.Z - 1); // Bottom

	for (int I = 0; I < FacesVisibility.Num(); I++)
	{
		if (FacesVisibility[I])
		{
			AddFaceData(Voxel, X, Y, Z, I);
		}
	}
}

bool UVoxelChunk::IsFaceVisible(int32 X, int32 Y, int32 Z) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	return VoxelWorld->IsVoxelTransparent(FIntVector(X, Y, Z));
}

void UVoxelChunk::AddFaceData(const Voxel& Voxel, int32 X, int32 Y, int32 Z, int FaceIndex)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);
	double VoxelSizeWorld = VoxelWorld->GetVoxelSizeWorld();
	checkSlow(DynamicMeshComponent);
	checkSlow(DynamicMeshComponent->GetDynamicMesh());
	FDynamicMesh3& Mesh = DynamicMeshComponent->GetDynamicMesh()->GetMeshRef();
	auto* UvLayer = Mesh.Attributes()->GetUVLayer(0);
	check(UvLayer);

	int32 VoxelTypeInt = static_cast<int32>(Voxel.VoxelType);
	int32 VoxelTypeNum = static_cast<int32>(EVoxelType::MAX);
	float UMin = static_cast<float>(FaceIndex) / 6;
	float UMax = static_cast<float>(FaceIndex + 1) / 6;
	float VMin = static_cast<float>(VoxelTypeInt) / VoxelTypeNum;
	float VMax = static_cast<float>(VoxelTypeInt + 1) / VoxelTypeNum;

	if (FaceIndex == 0) // Right
	{
		int Vid1 = Mesh.AppendVertex(FVector(X,		Y + 1, Z	) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X,		Y + 1, Z + 1) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X + 1, Y + 1, Z + 1) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X + 1, Y + 1, Z	) * VoxelSizeWorld);

		
		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}

	else if (FaceIndex == 1) // Left
	{
		int Vid1 = Mesh.AppendVertex(FVector(X,		Y,	Z		) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X + 1, Y,	Z		) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X + 1, Y,	Z + 1	) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X,		Y,	Z + 1	) * VoxelSizeWorld);
		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}

	else if (FaceIndex == 2) // Back
	{
		int Vid1 = Mesh.AppendVertex(FVector(X,	Y,		Z		) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X,	Y,		Z + 1	) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X, Y + 1,	Z + 1	) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X, Y + 1,	Z		) * VoxelSizeWorld);
		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}

	else if (FaceIndex == 3) // Front
	{
		int Vid1 = Mesh.AppendVertex(FVector(X + 1, Y,		Z + 1	) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X + 1, Y,		Z		) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X + 1, Y + 1,	Z		) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X + 1, Y + 1,	Z + 1	) * VoxelSizeWorld);

		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}

	else if (FaceIndex == 4) // Top
	{
		int Vid1 = Mesh.AppendVertex(FVector(X,		Y,		Z + 1) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X + 1, Y,		Z + 1) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X + 1, Y + 1,	Z + 1) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X,		Y + 1,	Z + 1) * VoxelSizeWorld);

		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}

	else if (FaceIndex == 5) // Bottom
	{
		int Vid1 = Mesh.AppendVertex(FVector(X + 1,	Y,		Z) * VoxelSizeWorld);
		int Vid2 = Mesh.AppendVertex(FVector(X,		Y,		Z) * VoxelSizeWorld);
		int Vid3 = Mesh.AppendVertex(FVector(X,		Y + 1,	Z) * VoxelSizeWorld);
		int Vid4 = Mesh.AppendVertex(FVector(X + 1, Y + 1,	Z) * VoxelSizeWorld);
		Mesh.SetVertexUV(Vid1, FVector2f(UMin, VMax));
		Mesh.SetVertexUV(Vid2, FVector2f(UMin, VMin));
		Mesh.SetVertexUV(Vid3, FVector2f(UMax, VMin));
		Mesh.SetVertexUV(Vid4, FVector2f(UMax, VMax));
	}
	AddTriangleIndices();
}

void UVoxelChunk::AddTriangleIndices()
{
	checkSlow(DynamicMeshComponent);
	checkSlow(DynamicMeshComponent->GetDynamicMesh());
	FDynamicMesh3& Mesh = DynamicMeshComponent->GetDynamicMesh()->GetMeshRef();

	int VertCount = Mesh.VertexCount();

	// Mesh.AppendTriangle(VertCount - 4, VertCount - 3, VertCount - 2);
	Mesh.AppendTriangle(VertCount - 2, VertCount - 3, VertCount - 4);
	Mesh.AppendTriangle(VertCount - 1, VertCount - 2, VertCount - 4);
}

// Called every frame
void UVoxelChunk::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDebugDrawDimensions)
	{
		AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
		checkSlow(VoxelWorld);
		FBox3d Bbox = GetWorldBoundingBox();
		DrawDebugBox(GetWorld(), Bbox.GetCenter(), Bbox.GetExtent(), FColor::Green);
	}
}

int32 UVoxelChunk::GetChunkSide() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);
	return VoxelWorld->GetChunkSide();
}

void UVoxelChunk::GetChunkIndex(int32& OutX, int32& OutY) const
{
	OutX = ChunkX;
	OutY = OutY;
}

void UVoxelChunk::SetChunkIndex(int32 X, int32 Y)
{
	ChunkX = X;
	ChunkY = Y;
}

FBox UVoxelChunk::GetWorldBoundingBox() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	double VoxelSizeWorld = VoxelWorld->GetVoxelSizeWorld();

	FIntVector MinVoxel;
	FIntVector MaxVoxel;
	GetVoxelBoundingBox(MinVoxel, MaxVoxel);

	FVector Min = VoxelWorld->GetActorLocation() + FVector(MinVoxel.X, MinVoxel.Y, MinVoxel.Z) * VoxelSizeWorld;
	FVector Max = VoxelWorld->GetActorLocation() + FVector(MaxVoxel.X, MaxVoxel.Y, MaxVoxel.Z) * VoxelSizeWorld;
	return FBox3d(Min, Max);
}

void UVoxelChunk::GetVoxelBoundingBox(FIntVector& OutMin, FIntVector& OutMax) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 WorldHeightVoxel = VoxelWorld->GetWorldHeight();
	int32 FirstBlockX = ChunkX * ChunkSide;
	int32 FirstBlockY = ChunkY * ChunkSide;
	int32 FirstBlockZ = 0;
	
	OutMin = FIntVector(FirstBlockX, FirstBlockY, FirstBlockZ);
	OutMax = FIntVector(FirstBlockX + ChunkSide, FirstBlockY + ChunkSide, FirstBlockZ + WorldHeightVoxel);
}

void UVoxelChunk::SetDrawWireframe(bool bEnabled)
{
	check(DynamicMeshComponent);
	DynamicMeshComponent->SetEnableWireframeRenderPass(bEnabled);
	bDebugDrawDimensions = bEnabled;
}

bool UVoxelChunk::GetDrawWireframe() const
{
	return bDebugDrawDimensions;
}

