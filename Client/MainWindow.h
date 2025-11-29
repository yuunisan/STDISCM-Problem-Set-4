#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout> 
#include <QFileDialog>
#include <QThread>
#include <QScrollArea>
#include <QMessageBox> 

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ocr_service::OCR;
using ocr_service::ImageRequest;
using ocr_service::OCRResult;

class OCRWorker : public QThread {
    Q_OBJECT
public:
    OCRWorker(std::shared_ptr<OCR::Stub> stub, QString filePath, int listRowId)
        : stub_(stub), filePath_(filePath), rowId_(listRowId) {
    }

    void run() override {
        ImageRequest request;
        OCRResult reply;
        ClientContext context;

        QFile file(filePath_);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray bytes = file.readAll();
            request.set_filename(filePath_.toStdString());
            request.set_image_data(bytes.toStdString());
        }
        else {
            emit resultReady(rowId_, "File cannot be read.", 0);
            return;
        }

        Status status = stub_->ProcessImage(&context, request, &reply);

        if (status.ok()) {
            emit resultReady(rowId_, QString::fromStdString(reply.extracted_text()), reply.processing_time_ms());
        }
        else {
            emit resultReady(rowId_, "Error detected: " + QString::fromStdString(status.error_message()), 0);
        }
    }

signals:
    void resultReady(int rowId, QString text, int timeMs);

private:
    std::shared_ptr<OCR::Stub> stub_;
    QString filePath_;
    int rowId_;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void uploadImages();
    void handleResult(int rowId, QString text, int timeMs);

private:
    std::shared_ptr<Channel> channel_;
    std::shared_ptr<OCR::Stub> stub_;

    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QLabel* ipLabel;
    QPushButton* uploadBtn;
    QProgressBar* progressBar;
    QScrollArea* scrollArea;
    QWidget* gridContainer;
    QGridLayout* gridLayout;

    int totalImages = 0;
    int processedImages = 0;
    int currentRow = 0;
};