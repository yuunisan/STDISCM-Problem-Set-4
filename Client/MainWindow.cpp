#include "MainWindow.h"
#include <QGroupBox>
#include <QPixmap>
#include <QFileInfo>
#include <QMessageBox>

// server ip change when using VM pls
const QString SERVER_ADDRESS = "127.0.0.1:50051";

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Distributed OCR Client");
    resize(950, 750);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);

    QWidget* topContainer = new QWidget();
    QHBoxLayout* topLayout = new QHBoxLayout(topContainer);

    ipLabel = new QLabel("Server: " + SERVER_ADDRESS);
    ipLabel->setStyleSheet("color: #555; font-style: bold; margin-right: 10px;");

    uploadBtn = new QPushButton("Upload Images", this);

    progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");

    topLayout->addWidget(ipLabel);
    topLayout->addWidget(uploadBtn);
    topLayout->addWidget(progressBar);
    mainLayout->addWidget(topContainer);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    gridContainer = new QWidget();
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(gridContainer);
    mainLayout->addWidget(scrollArea);

    connect(uploadBtn, &QPushButton::clicked, this, &MainWindow::uploadImages);

    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(50 * 1024 * 1024); // limit message size to 50mbb
    args.SetMaxSendMessageSize(50 * 1024 * 1024);

    channel_ = grpc::CreateCustomChannel(
        SERVER_ADDRESS.toStdString(),
        grpc::InsecureChannelCredentials(),
        args
    );
    stub_ = OCR::NewStub(channel_);
}

MainWindow::~MainWindow() {}

void MainWindow::uploadImages() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this);

    if (fileNames.isEmpty()) return;

    // checker if previous batch is done --> new session in uploading (reset progress bar, clean widget too)
    if (processedImages > 0 && processedImages == totalImages) {
        QLayoutItem* item;
        while ((item = gridLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        totalImages = 0;
        processedImages = 0;
        currentRow = 0;
        progressBar->setValue(0);
    }

    int col = 0;
    int maxCols = 4;

    if (totalImages > 0) {
        int existingItems = totalImages;
        currentRow = existingItems / maxCols;
        col = existingItems % maxCols;
    }

    for (const QString& filePath : fileNames) {
        QGroupBox* card = new QGroupBox();
        card->setFixedSize(200, 180);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(5, 5, 5, 5);

        QLabel* imgLabel = new QLabel();
        imgLabel->setAlignment(Qt::AlignCenter);
        QPixmap pixmap(filePath);
        imgLabel->setPixmap(pixmap.scaled(180, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QLabel* nameLabel = new QLabel(QFileInfo(filePath).fileName());
        nameLabel->setAlignment(Qt::AlignCenter);
        QFont font = nameLabel->font();
        font.setPointSize(8);
        nameLabel->setFont(font);

        QLabel* resultLabel = new QLabel("Processing...");
        resultLabel->setAlignment(Qt::AlignCenter);

        int myId = totalImages;
        resultLabel->setProperty("id", myId);

        cardLayout->addWidget(imgLabel);
        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(resultLabel);

        gridLayout->addWidget(card, currentRow, col);

        col++;
        if (col >= maxCols) {
            col = 0;
            currentRow++;
        }

        // ocr worker starts processing image
        OCRWorker* worker = new OCRWorker(stub_, filePath, myId);
        connect(worker, &OCRWorker::resultReady, this, &MainWindow::handleResult);
        connect(worker, &OCRWorker::finished, worker, &QObject::deleteLater);
        worker->start();

        totalImages++;
    }

    // per upload, recalculate the progress bar too, if new images appended --> update progress bar with new max imgs
    progressBar->setMaximum(totalImages);
}

void MainWindow::handleResult(int rowId, QString text, int timeMs) {
    QList<QLabel*> labels = gridContainer->findChildren<QLabel*>();

    for (QLabel* label : labels) {
        if (label->property("id").isValid() && label->property("id").toInt() == rowId) {

            if (timeMs > 0) {
                label->setText(text);
                QFont f = label->font();
                f.setBold(true);
                label->setFont(f);
                label->setToolTip(text);
            }
            else {
                label->setText(text);
            }
            break;
        }
    }
    processedImages++;
    progressBar->setValue(processedImages);
}