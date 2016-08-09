// Copyright (C) 2016 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <memory>
#include "BlendStateD3D11.h"
#include "RendererD3D11.h"
#include "core/Engine.h"
#include "utils/Utils.h"

namespace ouzel
{
    namespace graphics
    {
        BlendStateD3D11::BlendStateD3D11():
            dirty(false)
        {

        }

        BlendStateD3D11::~BlendStateD3D11()
        {
            if (blendState)
            {
                blendState->Release();
            }
        }

        void BlendStateD3D11::free()
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            BlendState::free();

            if (blendState)
            {
                blendState->Release();
                blendState = nullptr;
            }
        }

        bool BlendStateD3D11::init(bool enableBlending,
                                   BlendFactor colorBlendSource, BlendFactor colorBlendDest,
                                   BlendOperation colorOperation,
                                   BlendFactor alphaBlendSource, BlendFactor alphaBlendDest,
                                   BlendOperation alphaOperation)
        {
            free();

            std::lock_guard<std::mutex> lock(dataMutex);

            if (!BlendState::init(enableBlending,
                                  colorBlendSource, colorBlendDest,
                                  colorOperation,
                                  alphaBlendSource, alphaBlendDest,
                                  alphaOperation))
            {
                return false;
            }

            dirty = true;

            sharedEngine->getRenderer()->scheduleUpdate(shared_from_this());

            return true;
        }

        D3D11_BLEND BlendStateD3D11::getBlendFactor(BlendFactor blendFactor)
        {
            switch (blendFactor)
            {
                case BlendFactor::ZERO: return D3D11_BLEND_ZERO;
                case BlendFactor::ONE: return D3D11_BLEND_ONE;
                case BlendFactor::SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
                case BlendFactor::INV_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
                case BlendFactor::SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
                case BlendFactor::INV_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DEST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
                case BlendFactor::INV_DEST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
                case BlendFactor::DEST_COLOR: return D3D11_BLEND_DEST_COLOR;
                case BlendFactor::INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
                case BlendFactor::SRC_ALPHA_SAT: return D3D11_BLEND_SRC_ALPHA_SAT;
                case BlendFactor::BLEND_FACTOR: return D3D11_BLEND_BLEND_FACTOR;
                case BlendFactor::INV_BLEND_FACTOR: return D3D11_BLEND_INV_BLEND_FACTOR;
            }

            return D3D11_BLEND_ZERO;
        }

        D3D11_BLEND_OP BlendStateD3D11::getBlendOperation(BlendOperation blendOperation)
        {
            switch (blendOperation)
            {
                case BlendOperation::ADD: return D3D11_BLEND_OP_ADD;
                case BlendOperation::SUBTRACT: return D3D11_BLEND_OP_SUBTRACT;
                case BlendOperation::REV_SUBTRACT: return D3D11_BLEND_OP_REV_SUBTRACT;
                case BlendOperation::MIN: return D3D11_BLEND_OP_MIN;
                case BlendOperation::MAX: return D3D11_BLEND_OP_MAX;
            }

            return D3D11_BLEND_OP_ADD;
        }

        bool BlendStateD3D11::update()
        {
            if (dirty)
            {
                std::lock_guard<std::mutex> lock(dataMutex);

                std::shared_ptr<RendererD3D11> rendererD3D11 = std::static_pointer_cast<RendererD3D11>(sharedEngine->getRenderer());

                // Blending state
                D3D11_BLEND_DESC blendStateDesc = { FALSE, FALSE }; // alpha to coverage, independent blend
                D3D11_RENDER_TARGET_BLEND_DESC targetBlendDesc =
                {
                    enableBlending ? TRUE : FALSE,
                    getBlendFactor(colorBlendSource), getBlendFactor(colorBlendDest), getBlendOperation(colorOperation),
                    getBlendFactor(alphaBlendSource), getBlendFactor(alphaBlendDest), getBlendOperation(alphaOperation),
                    D3D11_COLOR_WRITE_ENABLE_ALL, // color write mask
                };
                blendStateDesc.RenderTarget[0] = targetBlendDesc;

                HRESULT hr = rendererD3D11->getDevice()->CreateBlendState(&blendStateDesc, &blendState);
                if (FAILED(hr))
                {
                    log("Failed to create Direct3D 11 blend state");
                    return false;
                }

                ready = true;
                dirty = false;
            }

            return true;
        }
    } // namespace graphics
} // namespace ouzel
