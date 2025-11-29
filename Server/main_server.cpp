#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <filesystem>

#include <grpcpp/grpcpp.h>
#include "ocr.grpc.pb.h"

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using ocr_service::OCR;
using ocr_service::ImageRequest;
using ocr_service::OCRResult;

// for tessdata pathfile (tessdata is located in Server folde r inside build fodler)
std::filesystem::path currentPath = std::filesystem::current_path();
std::filesystem::path tessPath = currentPath / "tessdata";
std::string pathStr = tessPath.string();

class OCRServiceImpl final : public OCR::Service {
public:
    Status ProcessImage(ServerContext* context, const ImageRequest* request, OCRResult* reply) override {
        std::cout << "Server is Processing: " << request->filename() << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        const std::string& imgData = request->image_data();
        if (imgData.empty()) return Status::CANCELLED;

        Pix* image = pixReadMem((const l_uint8*)imgData.data(), imgData.size());
        if (!image) return Status::CANCELLED;

        // debug filepath of tessdata if workin
        std::cout << "Server is looking for tessdata at filepath: " << pathStr << std::endl;

        tesseract::TessBaseAPI ocrApi;
        if (ocrApi.Init(pathStr.c_str(), "eng_fast", tesseract::OEM_DEFAULT)) {
            pixDestroy(&image);
            return Status::CANCELLED;
        }
        ocrApi.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
        Pix* gray = pixConvertTo8(image, false);
        Pix* blurred = pixRankFilter(gray, 3, 3, 0.5);
        Pix* binary = pixThresholdToBinary(blurred, 128);

        ocrApi.SetImage(binary);
        char* text = ocrApi.GetUTF8Text();
        std::string result_text = text ? text : "";

        while (!result_text.empty() && (result_text.back() == '\n' || result_text.back() == '\r')) {
            result_text.pop_back();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        reply->set_filename(request->filename());
        reply->set_extracted_text(result_text);
        reply->set_processing_time_ms((int)duration);

        delete[] text;
        pixDestroy(&image); pixDestroy(&gray); pixDestroy(&blurred); pixDestroy(&binary);

        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    OCRServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server is listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}