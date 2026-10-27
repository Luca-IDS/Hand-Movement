/*!
 * \file    acquisitionworker.cpp
 * \author  IDS Imaging Development Systems GmbH
 * \date    2022-06-01
 * \since   1.0.0
 *
 * \brief   The AcquisitionWorker class is used in a worker thread to capture
 *          images from the device continuously and do an image conversion into
 *          a desired pixel format.
 *
 * \version 1.2
 *
 * Copyright (C) 2019 - 2026, IDS Imaging Development Systems GmbH.
 *
 * The information in this document is subject to change without notice
 * and should not be construed as a commitment by IDS Imaging Development Systems GmbH.
 * IDS Imaging Development Systems GmbH does not assume any responsibility for any errors
 * that may appear in this document.
 *
 * This document, or source code, is provided solely as an example of how to utilize
 * IDS Imaging Development Systems GmbH software libraries in a sample application.
 * IDS Imaging Development Systems GmbH does not assume any responsibility
 * for the use or reliability of any portion of this document.
 *
 * General permission to copy or modify is hereby granted.
 */

#include "acquisitionworker.h"

#include <peak_icv/peak_icv.hpp>

#include <QDebug>

#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include "hand.h"
#include <QDir>
#include <QString>
#include "frame_overlay.h"

AcquisitionWorker::AcquisitionWorker(QObject* parent)
    : QObject(parent)
{
    m_running = false;
    m_frameCounter = 0;
    m_errorCounter = 0;
}

void AcquisitionWorker::Start()
{
    try
    {
        // Lock writable nodes, which could influence the payload size or
        // similar information during acquisition.
        m_nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);

        // Determine image size
        m_imageWidth = m_nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Width")->Value();
        m_imageHeight = m_nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Height")->Value();

        // Start acquisition both locally and on device.
        m_dataStream->StartAcquisition();
        m_nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();
        m_nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->WaitUntilDone();
    }
    catch (const std::exception& e)
    {
        qDebug() << "Exception: " << e.what();
    }

    peak::pipeline::DefaultPipeline pipeline;
    pipeline.SetOutputPixelFormat(peak::common::PixelFormat::BGRa8);

    m_running = true;
    while (m_running)
    {
        try
        {
            // Get buffer from device's datastream
            const auto buffer = m_dataStream->WaitForFinishedBuffer(5000);
            //const auto image = pipeline.Process(buffer->ToImageView());
            const auto image = pipeline.Process(buffer->ToImageView());
            //const auto image = "C:/Users/las/CLionProjects/HandMovment/Test Images/Hand im Bild_image2d.bmp";

            QImage qImage(static_cast<int>(m_imageWidth), static_cast<int>(m_imageHeight), QImage::Format_RGB32);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            auto qImageSize = static_cast<size_t>(qImage.sizeInBytes());
#else
            auto qImageSize = static_cast<size_t>(qImage.byteCount());
#endif

            // While the pipeline.Process() function guarantees making a copy,
            // we still copy over the image data into the QImage so we
            // we can release `image` while only keeping the `qImage`.
            std::memcpy(qImage.bits(), image.GetData(), qImageSize);

            // Re-queue the buffer so that it can be used by the datastream to receive a new image
            m_dataStream->QueueBuffer(buffer);

            // Emit signal that the image is ready to be displayed
            emit imageReceived(qImage);


            Hand hand1;
            FELD_MAPP mapp = hand1.serch_hand(buffer);
            FrameOverlay::saveFrameWithFields(qImage, mapp, m_frameCounter);



            m_frameCounter++;

        }
        catch (const std::exception& e)
        {
            m_errorCounter++;

            qDebug() << "Exception: " << e.what();
        }

        // Send signal with current frame and error counter
        emit counterChanged(m_frameCounter, m_errorCounter);


    }
}

void AcquisitionWorker::Stop()
{
    m_running = false;
}

void AcquisitionWorker::SetDataStream(std::shared_ptr<peak::core::DataStream> dataStream)
{
    m_dataStream = dataStream;
    m_nodemapRemoteDevice = m_dataStream->ParentDevice()->RemoteDevice()->NodeMaps().at(0);
}
