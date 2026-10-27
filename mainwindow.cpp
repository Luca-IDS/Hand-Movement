#include "mainwindow.h"

#include "acquisitionworker.h"
#include "display.h"

#include <peak/peak.hpp>
#include <peak_icv/peak_icv.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>
#include <QWidget>

#include <cstdint>

#define VERSION "1.2"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* widget = new QWidget(this);
    m_layout = new QVBoxLayout;
    widget->setLayout(m_layout);
    setCentralWidget(widget);

    // The library must be initialized before use.
    // Each `Initialize` call must be matched with a corresponding call
    // to `Close`.
    peak::Library::Initialize();
    peak::icv::library::Init();

    if (OpenDevice())
    {
        try
        {
            // Create a display for the camera image
            m_display = new CustomGraphicsView(widget);
            m_layout->addWidget(m_display);

            // Create worker thread that waits for new images from the camera
            m_acquisitionWorker = new AcquisitionWorker();
            m_acquisitionWorker->SetDataStream(m_dataStream);
            m_acquisitionWorker->moveToThread(&m_acquisitionThread);

            // Worker must be started, when the acquisition starts, and deleted, when the worker thread finishes
            connect(&m_acquisitionThread, &QThread::started, m_acquisitionWorker, &AcquisitionWorker::Start);
            connect(&m_acquisitionThread, &QThread::finished, m_acquisitionWorker, &QObject::deleteLater);

            // Connect the signal from the worker thread when a new image was received with the display update slot in
            // the Display class
            connect(m_acquisitionWorker, &AcquisitionWorker::imageReceived, m_display, &CustomGraphicsView::onImageReceived);

            // Connect the signal from the worker thread when the counters have changed with the update slot in the
            // MainWindow class
            connect(m_acquisitionWorker, &AcquisitionWorker::counterChanged, this, &MainWindow::onCounterChanged);

            // Start thread execution
            m_acquisitionThread.start();
        }
        catch (const std::exception& e)
        {
            QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        }
    }
    else
    {
        DestroyAll();
        exit(0);
    }

    createStatusBar();

    // Set minimum window size
    setMinimumSize(700, 500);
}


MainWindow::~MainWindow()
{
    DestroyAll();
}


void MainWindow::DestroyAll()
{
    if (m_acquisitionWorker)
    {
        m_acquisitionWorker->Stop();
        m_acquisitionThread.quit();
        m_acquisitionThread.wait();
    }

    CloseDevice();

    // Each `Initialize` call must be matched with a corresponding call to `Close`.
    peak::Library::Close();
    peak::icv::library::Exit();
}


bool MainWindow::OpenDevice()
{
    try
    {
        // Get the instance of the device manager singleton.
        auto& deviceManager = peak::DeviceManager::Instance();

        // Update the device manager.
        // When `Update` is called, it searches for all producer libraries
        // contained in the directories found in the official GenICam GenTL
        // environment variable GENICAM_GENTL{32/64}_PATH. It then opens all
        // found ProducerLibraries, their Systems, their Interfaces, and lists
        // all available DeviceDescriptors.
        deviceManager.Update();

        // Return if no device was found.
        if (deviceManager.Devices().empty())
        {
            QMessageBox::critical(this, "Error", "No device found", QMessageBox::Ok);
            return false;
        }

        // Open the first openable device in the device manager's device list.
        size_t deviceCount = deviceManager.Devices().size();
        for (size_t i = 0; i < deviceCount; ++i)
        {
            if (deviceManager.Devices().at(i)->IsOpenable())
            {
                m_device = deviceManager.Devices().at(i)->OpenDevice(peak::core::DeviceAccessType::Control);

                // Stop after the first opened device
                break;
            }
            else if (i == (deviceCount - 1))
            {
                QMessageBox::critical(this, "Error", "Device could not be opened", QMessageBox::Ok);
                return false;
            }
        }

        if (m_device)
        {
            auto dataStreams = m_device->DataStreams();
            if (dataStreams.empty())
            {
                QMessageBox::critical(this, "Error", "Device has no DataStream", QMessageBox::Ok);
                m_device.reset();
                return false;
            }

            try
            {
                // Open standard data stream
                m_dataStream = dataStreams.at(0)->OpenDataStream();
            }
            catch (const std::exception& e)
            {
                // Open data stream failed
                m_device.reset();
                QMessageBox::critical(
                    this, "Error", QString("Failed to open DataStream\n") + e.what(), QMessageBox::Ok);
                return false;
            }

            // Retrieve the remote device's primary node map, which in GenICam represents
            // a hierarchical set of device parameters (features) such as exposure, gain,
            // and firmware info. The remote nodemap provides access to controls implemented
            // on the device itself, primarily following the
            // GenICam Standard Feature Naming Convention (SFNC),
            // while also supporting custom nodes to accommodate device-specific features.
            m_nodemapRemoteDevice = m_device->RemoteDevice()->NodeMaps().at(0);

            // To prepare for untriggered continuous image acquisition, load the default user set if available
            // and wait until execution is finished
            try
            {
                m_nodemapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("UserSetSelector")
                    ->SetCurrentEntry("UserSet1");

                m_nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")->Execute();
                m_nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")
                    ->WaitUntilDone();

                //auto exposureAuto = m_nodemapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("ExposureAuto");
                //exposureAuto->SetCurrentEntry("On");

            }
            catch (const peak::core::NotFoundException&)
            {
                // UserSet is not available
            }

            // Get the payload size for correct buffer allocation
            auto payloadSize = m_nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")
                                   ->Value();

            // Get the minimum number of buffers that must be announced
            auto bufferCountMax = m_dataStream->NumBuffersAnnouncedMinRequired();

            // Allocate and announce image buffers and queue them
            for (size_t bufferCount = 0; bufferCount < bufferCountMax; ++bufferCount)
            {
                // Let the TL allocate the buffers.
                auto buffer = m_dataStream->AllocAndAnnounceBuffer(static_cast<size_t>(payloadSize), nullptr);
                m_dataStream->QueueBuffer(buffer);
            }

            return true;
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
    }

    return false;
}


void MainWindow::CloseDevice()
{
    // If device was opened, try to stop acquisition
    if (m_device)
    {
        try
        {
            auto remoteNodeMap = m_device->RemoteDevice()->NodeMaps().at(0);
            remoteNodeMap->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")->Execute();
        }
        catch (const std::exception& e)
        {
            QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        }
    }

    // If data stream was opened, try to stop it and revoke its image buffers
    if (m_dataStream)
    {
        try
        {
            // Stop and flush the `DataStream`.
            // `KillWait` will cancel pending `WaitForFinishedBuffer` calls.
            // NOTE: One call to `KillWait` will cancel one pending `WaitForFinishedBuffer`.
            //       For more information, refer to the documentation of `KillWait`.
            m_dataStream->KillWait();
            m_dataStream->StopAcquisition(peak::core::AcquisitionStopMode::Default);
            // Discard all buffers from the acquisition engine.
            // They remain in the announced buffer pool.
            m_dataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);

            for (const auto& buffer : m_dataStream->AnnouncedBuffers())
            {
                m_dataStream->RevokeBuffer(buffer);
            }
        }
        catch (const std::exception& e)
        {
            QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        }
    }

    if (m_nodemapRemoteDevice)
    {
        try
        {
            // Unlock parameters after acquisition stop
            m_nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);
        }
        catch (const std::exception& e)
        {
            QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        }
    }
}


void MainWindow::createStatusBar()
{
    auto statusBar = new QWidget(centralWidget());
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    m_labelInfo = new QLabel(statusBar);
    m_labelInfo->setAlignment(Qt::AlignLeft);
    layout->addWidget(m_labelInfo);
    layout->addStretch();

    auto m_labelVersion = new QLabel(statusBar);
    m_labelVersion->setText(("simple_live_qtwidgets v" VERSION));
    m_labelVersion->setAlignment(Qt::AlignRight);
    layout->addWidget(m_labelVersion);

    auto m_labelAboutQt = new QLabel(statusBar);
    m_labelAboutQt->setObjectName("aboutQt");
    m_labelAboutQt->setText(R"(<a href="#aboutQt">About Qt</a>)");
    m_labelAboutQt->setAlignment(Qt::AlignRight);
    connect(m_labelAboutQt, &QLabel::linkActivated, this, &MainWindow::onAboutQtLinkActivated);
    layout->addWidget(m_labelAboutQt);
    statusBar->setLayout(layout);

    m_layout->addWidget(statusBar);
}

void MainWindow::onCounterChanged(unsigned int frameCounter, unsigned int errorCounter)
{
    m_labelInfo->setText(QString("Frames acquired: %1, errors: %2")
            .arg(QString::number(frameCounter), QString::number(errorCounter)));
}

void MainWindow::onAboutQtLinkActivated(const QString& link)
{
    if (link == "#aboutQt")
    {
        QMessageBox::aboutQt(this, "About Qt");
    }
}
