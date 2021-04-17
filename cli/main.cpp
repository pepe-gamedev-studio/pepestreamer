#include <BroadcastController.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int Main(int argc, char *argv[]) {
#ifdef _DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#else
    av_log_set_level(AV_LOG_QUIET);
#endif

    avformat_network_init();

    AVCodecParameters *videoCodecParameters = avcodec_parameters_alloc();
    AVCodecParameters *audioCodecParameters = avcodec_parameters_alloc();

    videoCodecParameters->codec_id = AV_CODEC_ID_H264;
    videoCodecParameters->codec_tag = 0;
    videoCodecParameters->codec_type = AVMEDIA_TYPE_VIDEO;
    videoCodecParameters->format = AV_PIX_FMT_YUV420P;
    videoCodecParameters->width = 1920;
    videoCodecParameters->height = 1080;
    videoCodecParameters->bit_rate = 4500000;
    videoCodecParameters->sample_aspect_ratio = {1, 1};

    std::vector<uint8_t> audioExtraData{18, 16, 86, 229, 0};
    audioCodecParameters->codec_id = AV_CODEC_ID_AAC;
    audioCodecParameters->codec_tag = 0;
    audioCodecParameters->codec_type = AVMEDIA_TYPE_AUDIO;
    audioCodecParameters->bit_rate = 256000;
    audioCodecParameters->sample_rate = 44100;
    audioCodecParameters->frame_size = 1024;
    audioCodecParameters->channel_layout = AV_CH_LAYOUT_STEREO;
    audioCodecParameters->channels = av_get_channel_layout_nb_channels(audioCodecParameters->channel_layout);
    audioCodecParameters->extradata = audioExtraData.data();
    audioCodecParameters->extradata_size = static_cast<int>(audioExtraData.size());

    if (argc < 2) {
        std::cout << "Usage: cli [rtmp://host/path]\n";
        return 1;
    }

    pepestreamer::BroadcastController broadcaster;
    broadcaster.StartService({{videoCodecParameters, audioCodecParameters, argv[1]}, 25, 50, {3, 155, 229}});

    using namespace pepestreamer;

    std::map<std::string, std::unique_ptr<overlay::elements::TextElement, overlay::TestOverlay::TextElementDeleter>>
        textList;
    std::map<std::string, std::unique_ptr<overlay::elements::ImageElement, overlay::TestOverlay::ImageElementDeleter>>
        imageList;

    int64_t nextTextId = 0, nextImageId = 0;
    std::string cmd;
    while (true) {
        try {
            std::cout << ">> ";
            std::getline(std::cin, cmd);
            std::stringstream ss(cmd);

            std::string name;
            if (!(ss >> name))
                continue;

            if (name == "play") {
                std::string filename;
                if (ss >> filename) {
                    broadcaster.Play(filename);
                }
            } else if (name == "stop") {
                broadcaster.Stop();
            } else if (name == "seek") {
                int64_t position;
                if (ss >> position && position >= 0) {
                    broadcaster.Seek(position);
                }
            } else if (name == "drawtext") {
                std::string text, fontColor = "black";
                int16_t x = 0, y = 0, fontSize = 32;
                if (!(ss >> text))
                    continue;

                ss >> x >> y >> fontColor >> fontSize;

                const std::string id = "t" + std::to_string(nextTextId++);
                textList.insert({id, broadcaster.DrawText(text, x, y, fontColor, fontSize, "Roboto/Roboto-Black.ttf")});
            } else if (name == "drawimage") {
                std::string filename;
                int16_t x = 0, y = 0;
                if (!(ss >> filename))
                    continue;

                ss >> x >> y;

                const std::string id = "i" + std::to_string(nextImageId++);
                imageList.insert({id, broadcaster.DrawImage(filename, x, y)});
            } else if (name == "move") {
                std::string id;
                int32_t x, y;
                if (!(ss >> id >> x >> y))
                    throw std::runtime_error("Invalid input");

                switch (id[0]) {
                case 't': {
                    auto found = textList.find(id);
                    if (found != textList.end())
                        found->second->Position(x, y);
                    else
                        throw std::runtime_error("Failed to find text entity");
                    break;
                }
                case 'i': {
                    auto found = imageList.find(id);
                    if (found != imageList.end()) {
                        found->second->Position(x, y);
                    } else {
                        throw std::runtime_error("Failed to find image entity");
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Unknown entity type");
                }
            } else if (name == "remove") {
                std::string id;
                if (!(ss >> id))
                    throw std::runtime_error("Invalid input");

                switch (id[0]) {
                case 't': {
                    if (!textList.erase(id))
                        throw std::runtime_error("Failed to find image entity");
                    break;
                }
                case 'i': {
                    if (!imageList.erase(id))
                        throw std::runtime_error("Failed to find text entity");
                    break;
                }
                default:
                    throw std::runtime_error("Unknown entity type");
                }
            } else if (name == "list") {
                for (auto &x : imageList) {
                    std::cout << "id=" << x.first << " " << x.second->X() << " " << x.second->Y() << '\n';
                }

                for (auto &x : textList) {
                    std::cout << "id=" << x.first << " " << x.second->Text() << " " << x.second->X() << " "
                              << x.second->Y() << '\n';
                }
            } else if (name == "help") {
                std::cout << "play [filename]\n"
                          << "seek [seconds]\n"
                          << "stop \n"
                          << "drawtext [text] [x] [y] [fontColor] [fontSize]\n"
                          << "drawimage [filename] [x] [y]\n"
                          << "list\n"
                          << "move [id]\n"
                          << "remove [id]\n";
            } else {
                throw std::runtime_error("Unknown command");
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << '\n';
        }
    }

    std::cin.get();
    return 0;
}

#ifdef _WIN32
#include <Windows.h>
int wmain(int argc, wchar_t *argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    char **u8argv = new char *[argc];
    for (int argn = 0; argn < argc; ++argn) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argv[argn], -1, NULL, 0, NULL, NULL);

        u8argv[argn] = new char[size];

        WideCharToMultiByte(CP_UTF8, 0, argv[argn], -1, u8argv[argn], size, NULL, NULL);
    }

    Main(argc, u8argv);
}
#else
int main(int argc, char *argv[]) { Main(argc, argv); }
#endif