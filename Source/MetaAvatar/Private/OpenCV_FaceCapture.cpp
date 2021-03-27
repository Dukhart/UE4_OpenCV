// Copywrite Dukhart GNU v3.0
#include "OpenCV_FaceCapture.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"

// Sets default values
AOpenCV_FaceCapture::AOpenCV_FaceCapture(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    // ensure the root component exists
    if (!RootComponent)
        RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
    FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, false);
    // Initialize OpenCV and webcam properties
    CameraID = 0;
    VideoTrackID = 0;
    shouldReadFrame = false;
    isStreamOpen = false;
    VideoSize = FVector2D(1920, 1080);
}
AOpenCV_FaceCapture::~AOpenCV_FaceCapture() {
    cv::destroyWindow("OpenCV_Display");
}
// Called when the game starts or when spawned
void AOpenCV_FaceCapture::BeginPlay()
{
	Super::BeginPlay();
    // Prepare the color data array
    ColorData.AddDefaulted(VideoSize.X * VideoSize.Y);
    // create dynamic texture
    //OpenCV_Texture2D_Pre = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);
    OpenCV_Texture2D_Post = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
    //OpenCV_Texture2D_Pre->MipGenSettings = TMGS_NoMipmaps;
    OpenCV_Texture2D_Post->MipGenSettings = TMGS_NoMipmaps;
#endif
    //OpenCV_Texture2D_Pre->SRGB = RenderTarget_Raw->SRGB;
    OpenCV_Texture2D_Post->SRGB = RenderTarget_Raw->SRGB;

    LoadOpenCV();

    isStreamOpen = true;
}

void AOpenCV_FaceCapture::LoadOpenCV() {
    // setup openCV
    cvSize = cv::Size(VideoSize.X, VideoSize.Y);
    cvMat = cv::Mat(cvSize, CV_8UC4, ColorData.GetData());
    FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

    Path_Cascade_Face = ProjectPath + Path_Cascade_Face;
    Path_Cascade_Eyes = ProjectPath + Path_Cascade_Eyes;

    cvCascadeClassifier_Face = cv::CascadeClassifier(TCHAR_TO_UTF8(*Path_Cascade_Face));
    cvCascadeClassifier_Eyes = cv::CascadeClassifier(TCHAR_TO_UTF8(*Path_Cascade_Eyes));

    // load cascades
    if (!cvCascadeClassifier_Face.load(TCHAR_TO_UTF8(*Path_Cascade_Face)))
    {
        UE_LOG(LogTemp, Error, TEXT("Error loading face cascade\n"));
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Path_Cascade_Face);
        shouldReadFrame = false;
    };
    if (!cvCascadeClassifier_Eyes.load(TCHAR_TO_UTF8(*Path_Cascade_Eyes)))
    {
        UE_LOG(LogTemp, Error, TEXT("Error loading eyes cascade\n"));
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Path_Cascade_Eyes);
        shouldReadFrame = false;
    };
    shouldReadFrame = true;
}

// Called every frame
void AOpenCV_FaceCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (shouldReadFrame && isStreamOpen)
        ReadFrame();
}
bool AOpenCV_FaceCapture::ReadFrame() {
    if (!OpenCV_Texture2D_Pre || !RenderTarget_Raw) return false;
    // Read the pixels from the RenderTarget and store them in a FColor array
    //TArray<FColor> SurfData;
    
    // Get the color data
    cvMat = cv::Mat(cvSize, CV_8UC4, ColorData.GetData());

    // look for a face
   // if (!cvMat.empty() && FindFace()) {
        // show the openCV window
       // cv::imshow("OpenCV_Display", cvMat);
    //}

    //Wrapped in a render command for performance
    ENQUEUE_RENDER_COMMAND(WriteOpenCVTexture)(
        [RTarget_Raw = RenderTarget_Raw, RTexture_Pre = OpenCV_Texture2D_Pre, RTexture_Post = OpenCV_Texture2D_Post](FRHICommandList& RHICmdList)
        {
            TArray<FColor> ColorD;
            ColorD.AddDefaulted(RTarget_Raw->SizeX * RTarget_Raw->SizeY);
            //FTextureReference tref = RTarget_Raw->TextureReference;
            //FTextureReferenceRHIRef rhiref = tref.TextureReferenceRHI;
            //rhiref.
            //UTexture2D* RTexture_Pre = RTarget_Raw->ConstructTexture2D(RTarget_Raw, "AlphaTex", EObjectFlags::RF_NoFlags, CTF_DeferCompression);
            //This function will allocate memory for FormattedImageData pointer and then you can read from here all the pixels info
            //RTexture_Pre->PlatformData->Mips[0].BulkData.GetCopy((void**)&ColorD);
            //RTexture_Pre->PlatformData->Mips[0].BulkData.GetCopy((void**)&ColorD);
            void* TextureData1 = RTexture_Pre->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            void* TextureData2 = RTexture_Post->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            const int32 TextureDataSize = ColorD.Num() * 4;
            // set the texture data
            FMemory::Memcpy(TextureData1, TextureData2, TextureDataSize);
            //FMemory::Memcpy(TextureData, ColorD.GetData(), TextureDataSize);
            RTexture_Pre->PlatformData->Mips[0].BulkData.Unlock();
            RTexture_Post->PlatformData->Mips[0].BulkData.Unlock();
            // Apply Texture changes to GPU memory
            RTexture_Pre->UpdateResource();
            RTexture_Post->UpdateResource();
            RTarget_Raw->UpdateTexture2D(RTexture_Pre, TSF_BGRA8);
        });
    return true;
}

bool AOpenCV_FaceCapture::FindFace() {
    //cvCascadeClassifier_Face.load(TCHAR_TO_UTF8(*Path_Cascade_Face));
    //return false;
    cvMat_gray = cvMat;
    
    // Convert the image to grayscale
    cv::cvtColor(cvMat, cvMat_gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(cvMat_gray, cvMat_gray);
    //-- Detect faces
    
    cvCascadeClassifier_Face.detectMultiScale(cvMat_gray, faces);
    /*
    for (size_t i = 0; i < faces.size(); i++)
    {
        cv::Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
        cv::ellipse(mat, center, cv::Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, cv::Scalar(255, 0, 255), 4);
        cv::Mat faceROI = mat_gray(faces[i]);
        
        //-- In each face, detect eyes
        std::vector<Rect> eyes;
        cvCascadeClassifier_Eyes.detectMultiScale(faceROI, eyes);
        for (size_t j = 0; j < eyes.size(); j++)
        {
            cv::Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
            int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
            cv::circle(mat, eye_center, radius, cv::Scalar(255, 0, 0), 4);
        }
        
    }
    */
    return true;
}

//increment camera by one then validate exists
void AOpenCV_FaceCapture::NextCamera()
{
    CameraID += 1;
    ValidateCameraID();
}
//increment video track by one then validate exists
void AOpenCV_FaceCapture::NextVideoTrack()
{
    VideoTrackID += 1;
    ValidateVideoTrackID();
}