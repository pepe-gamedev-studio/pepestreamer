#include "TestOverlay.h"

#include "commands/DrawText.h"
#include "commands/Link.h"
#include "commands/Movie.h"
#include "commands/Overlay.h"

#include <cassert>

namespace pepestreamer::overlay {

TestOverlay::TestOverlay(const InputBufferParameters &sourceParameters, boost::mutex &m)
    : graph(sourceParameters), textNameGen("drawtext"), imageNameGen("movie"), overlayNameGen("overlay"), m(&m) {
    /**
     *				  ┌────┐ ┌───────────────┐ ┌─────┐
     *				  │ in ├─►               ├─► out │
     *  ┌─────────────┴────┤ │ canvasOverlay │ └─────┘
     *  │backgroundImage   ├─►               │
     *  └──────────────────┘ └───────────────┘
     */
    graph.Configure()
        .Add<commands::Movie>(backgroundImage, "background.png")
        .Add<commands::Overlay>(canvasOverlay, 0, 0)
        .Add<commands::Link>(in, 0, canvasOverlay, 0)
        .Add<commands::Link>(backgroundImage, 0, canvasOverlay, 1)
        .Add<commands::Link>(canvasOverlay, 0, out, 0)
        .Apply();
}

void TestOverlay::Push(AVFrame *frame) { graph.Push(frame); }

std::unique_ptr<elements::TextElement, TestOverlay::TextElementDeleter>
TestOverlay::DrawText(const std::string &text,
                      int16_t x,
                      int16_t y,
                      std::string fontColor,
                      int16_t fontSize,
                      const std::string &filename) {
    /**
     *                ┌────┐ ┌───────────────┐
     *                │ in ├─►               │ ┌────────────┐ ┌────────────┐  ┌──────┐
     *  ┌─────────────┴────┤ │ canvasOverlay ├─► drawtext_0 ├─► drawtext_1 ├─►  out  │
     *  │backgroundImage   ├─►               │ └────────────┘ └────────────┘  └──────┘
     *  └──────────────────┘ └───────────────┘
     */
    assert(graph.ManagedGraph()->Out()->nb_inputs == 1);

    AVFilterLink *insertionPoint = graph.ManagedGraph()->Out()->inputs[0];
    const std::string leftNodeName = insertionPoint->src->name;

    const std::string textName = textNameGen.Generate();
    auto configurator = graph.Configure()
                            .Remove(commands::Link::GetName(insertionPoint))
                            .Add<commands::DrawText>(textName, text, x, y, fontColor, fontSize, filename)
                            .Add<commands::Link>(leftNodeName, 0, textName, 0)
                            .Add<commands::Link>(textName, 0, out, 0);

    {
        boost::lock_guard<boost::mutex> lock(*m);

        configurator.Apply();
    }

    return std::unique_ptr<elements::TextElement, TextElementDeleter>(
        new elements::TextElement(graph.FindRecipeItem<commands::DrawText>(textName), graph.ManagedGraph()),
        [&](elements::TextElement *ptr) {
            auto configurator = graph.Configure();
            auto links = graph.GetFilterLinks(ptr->Command());

            assert(links.size() == 2);

            auto command = ptr->Command().lock();
            auto firstLink = links.front().lock();
            auto secondLink = links.back().lock();
            // First link connected at left
            if (firstLink->destination == command->Name()) {
                configurator.Add<commands::Link>(firstLink->source, 0, secondLink->destination, 0);
            } else {
                configurator.Add<commands::Link>(secondLink->source, 0, firstLink->destination, 0);
            }

            for (auto &link : links) {
                configurator.Remove(link.lock()->Name());
            }

            configurator.Remove(command->Name());

            {
                boost::lock_guard<boost::mutex> lock(*m);

                configurator.Apply();
            }

            delete ptr;
        });
}

std::unique_ptr<elements::ImageElement, TestOverlay::ImageElementDeleter>
TestOverlay::DrawImage(const std::string &filename, int16_t x, int16_t y) {
    /**
     *                              ┌────┐ ┌───────────────┐
     *                              │ in ├─►               │ ┌─────┐
     *  ┌──────────────────┐ ┌──────┴────┤ │ canvasOverlay ├─► out │
     *  │ backgroundImage  ├─►           ├─►               │ └─────┘
     *  └───────┬──────────┤ │ overlay_0 │ └───────────────┘
     *          │  movie_0 ├─►           │
     *          └──────────┘ └───────────┘
     */
    using LinkPtr = std::weak_ptr<commands::Link>;

    auto canvasRecipe = graph.FindRecipeItem<commands::Overlay>(canvasOverlay);
    auto canvasLinks = graph.GetFilterLinks(canvasRecipe);
    auto left =
        std::find_if(canvasLinks.begin(), canvasLinks.end(), [&](LinkPtr &x) { return x.lock()->destinationPad == 1; });

    assert(left != canvasLinks.end());

    auto l = left->lock();

    const std::string imageName = imageNameGen.Generate();
    const std::string overlayName = overlayNameGen.Generate();
    auto cinfigurator = graph.Configure()
                            .Remove(l->Name())
                            .Add<commands::Movie>(imageName, filename)
                            .Add<commands::Overlay>(overlayName, x, y)
                            .Add<commands::Link>(imageName, 0, overlayName, 1)
                            .Add<commands::Link>(overlayName, 0, canvasOverlay, 1)
                            .Add<commands::Link>(l->source, 0, overlayName, 0);

    {
        boost::lock_guard<boost::mutex> lock(*m);

        cinfigurator.Apply();
    }

    return std::unique_ptr<elements::ImageElement, ImageElementDeleter>(
        new elements::ImageElement(graph.FindRecipeItem<commands::Overlay>(overlayName), &graph, m),
        [&, imageName, overlayName](elements::ImageElement *ptr) {
            auto configurator = graph.Configure();
            auto links = graph.GetFilterLinks(ptr->Command());

            assert(links.size() == 3);

            auto prevSourceLink = std::find_if(links.begin(), links.end(), [&](LinkPtr &x) {
                auto l = x.lock();

                return l->destinationPad == 0 && l->destination == overlayName;
            });

            auto prevDestinationLink = std::find_if(links.begin(), links.end(), [&](LinkPtr &x) {
                auto l = x.lock();

                return l->sourcePad == 0 && l->source == overlayName;
            });

            const std::string sourceName = prevSourceLink->lock()->source;
            const std::string destinationName = prevDestinationLink->lock()->destination;

            configurator.Remove(imageName).Remove(overlayName);

            for (auto &x : links) {
                configurator.Remove(x.lock()->Name());
            }

            configurator.Add<commands::Link>(sourceName, 0, destinationName, destinationName == canvasOverlay);

            {
                boost::lock_guard<boost::mutex> lock(*m);

                configurator.Apply();
            }

            delete ptr;
        });
}

void TestOverlay::Reader(ReadDelegate reader) { graph.Reader(reader); }

} // namespace pepestreamer::overlay