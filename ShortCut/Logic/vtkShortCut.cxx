#include <iostream>
#include "vtkShortCut.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "itkImage.h"
#include "itkTimeProbe.h"

#include "ShortCutSegmenter.h"
#include "TCGAUtilities.h"

//vtkCxxRevisionMacro(vtkFastGrowCutSeg, "$Revision$"); //necessary?
vtkStandardNewMacro(vtkShortCut); //for the new() macro

//----------------------------------------------------------------------------
vtkShortCut::vtkShortCut() {

    SourceVol = NULL;
    SeedVol = NULL;
    SegVol = NULL;
    SCROIVol = NULL;
    m_qTCGASeg = NULL;

    otsuRatio = 1.0;
    curvatureWeight = 0.8;
    sizeThld = 3;
    sizeUpperThld = 200;
    mpp = 0.25;
}


vtkShortCut::~vtkShortCut() {

    //these functions decrement reference count on the vtkImageData's (incremented by the SetMacros)
    if (this->SourceVol) {
        this->SetSourceVol(NULL);
    }
    if (this->SegVol) {
        this->SetSegVol(NULL);
    }

    if (this->SeedVol) {
        this->SetSeedVol(NULL);
    }
    if (this->SCROIVol) {
        this->SetSCROIVol(NULL);
    }
    if (m_qTCGASeg) {
        delete m_qTCGASeg;
    }
}

void vtkShortCut::Initialization() {

    std::cout << "vtkShortCut initialized\n";
    InitializationFlag = false;
    if (m_qTCGASeg == NULL) {
        m_qTCGASeg = new ShortCutSegmenter();
    }
}


void vtkShortCut::Run_QTCGA_Segmentation() {
    std::cout << "vtkShortCut::Run_QTCGA_Segmentation\n";

    //Convert vtkImage to lplImage
    int dims[3];
    SourceVol->GetDimensions(dims);
    m_imSrc = cv::Mat(dims[0], dims[1], CV_8UC3); // arguments 1 & 2 reverse of vtkQuickTCGA
    m_imLab = cv::Mat(dims[0], dims[1], CV_8UC1); // ditto

    TCGA::CopyImageVTK2OpenCV<uchar, uchar>(SourceVol, m_imSrc);
    TCGA::CopyImageVTK2OpenCV<short, uchar>(SeedVol, m_imLab);

    m_qTCGASeg->SetSourceImage(m_imSrc);
    m_qTCGASeg->SetLabImage(m_imLab);
    std::cout << "HERE. DoSegmentation\n";

    m_qTCGASeg->DoSegmentation();
    std::cout << "4\n";

    m_qTCGASeg->GetSegmentation(m_imLab);
    std::cout << "5\n";

    // Convert lplImage to vtkImage and update SeedVol
    TCGA::CopyImageOpenCV2VTK<uchar, short>(m_imLab, SeedVol);

    std::cout << "Finished TCGA segmentation\n";
}

/**
 * Run_QTCGA_ShortCut
 */
void vtkShortCut::Run_QTCGA_ShortCut() {

    //Convert vtkImage to lplImage
    int dims[3];
    SourceVol->GetDimensions(dims);
    m_imSrc = cv::Mat(dims[0], dims[1], CV_8UC3); // arguments 1 & 2 reverse of vtkQuickTCGA
    m_imLab = cv::Mat(dims[0], dims[1], CV_8UC1); // ditto
    m_imSCROI = cv::Mat(dims[0], dims[1], CV_8UC1);

    TCGA::CopyImageVTK2OpenCV<uchar, uchar>(SourceVol, m_imSrc);
    TCGA::CopyImageVTK2OpenCV<short, uchar>(SeedVol, m_imLab);
    TCGA::CopyImageVTK2OpenCV<short, uchar>(SCROIVol, m_imSCROI);

    m_qTCGASeg->SetSourceImage(m_imSrc);
    m_qTCGASeg->SetLabImage(m_imLab);
    m_qTCGASeg->SetROIImage(m_imSCROI);

    m_qTCGASeg->RefineShortCut();

    m_qTCGASeg->GetSegmentation(m_imLab);

    // Convert lplImage to vtkImage and update SeedVol
    TCGA::CopyImageOpenCV2VTK<uchar, short>(m_imLab, SeedVol);

    std::cout << "Finished TCGA ShortCut\n";
}

void vtkShortCut::Run_Refine_Curvature() {
    //Convert vtkImage to lplImage
    int dims[3];
    SourceVol->GetDimensions(dims);
    m_imSrc = cv::Mat(dims[0], dims[1], CV_8UC3); // arguments 1 & 2 reverse of vtkQuickTCGA
    m_imLab = cv::Mat(dims[0], dims[1], CV_8UC1); // ditto

    TCGA::CopyImageVTK2OpenCV<uchar, uchar>(SourceVol, m_imSrc);
    TCGA::CopyImageVTK2OpenCV<short, uchar>(SeedVol, m_imLab);

    m_qTCGASeg->SetSourceImage(m_imSrc);
    m_qTCGASeg->SetLabImage(m_imLab);

    m_qTCGASeg->RefineCurvature();

    m_qTCGASeg->GetSegmentation(m_imLab);

    // Convert lplImage to vtkImage and update SeedVol
    TCGA::CopyImageOpenCV2VTK<uchar, short>(m_imLab, SeedVol);

    std::cout << "Finished TCGA refinement\n";
}

void vtkShortCut::Run_QTCGA_Template() {
    std::cout << "Finished TCGA template matching\n";
}

void vtkShortCut::PrintSelf(ostream &os, vtkIndent indent) {
    std::cout << "This function has been found" << std::endl;
}
