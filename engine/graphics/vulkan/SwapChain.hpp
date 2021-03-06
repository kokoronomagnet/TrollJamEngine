#pragma once
#include <engine/graphics/vulkan/defines.hpp>
//#include <engine/graphics/vulkan/Device.hpp>
#include <engine/os/Window.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#undef min
#undef max
namespace VK
{
	class SwapChain
	{
		VK_OBJECT( SwapChain );
	private:
		struct Surface
		{
			Unique< VkSurfaceKHR > handle;
			VkSurfaceCapabilitiesKHR capabilities;
			LocalArray< VkPresentModeKHR , 20 > present_modes;
			LocalArray< VkSurfaceFormatKHR , 20 > formats;
			VkSurfaceFormatKHR preffered_format;
			VkPresentModeKHR preffered_present_mode;
		} surface;
		VkDevice dev_raw;
		Unique< VkSwapchainKHR > handle;
		LocalArray< Image , 10 > attachment_images;
		LocalArray< ImageView , 10 > attachment_image_views;
		uint attachment_count;
		uint32_t current_image = 0;
		uint width , height;
	public:
		VkSwapchainKHR const &getHandle() const
		{
			return *handle;
		}
		uint32_t const &getCurrentImageIndex() const
		{
			return current_image;
		}
		VkFormat getFormat() const
		{
			return surface.preffered_format.format;
		}
		int2 getSize() const
		{
			return{ width , height };
		}
		void acquireNextImage( VkSemaphore signal_semaphore )
		{
			VKASSERTLOG( vkAcquireNextImageKHR(
				dev_raw , *handle , UINT64_MAX ,
				signal_semaphore , VK_NULL_HANDLE , &current_image
			) );
		}
		uint32_t getImagesCount() const
		{
			return attachment_count;
		}
		auto const &getCurrentAttachment() const
		{
			return attachment_images[ current_image ];
		}
		auto const &getCurrentAttachmentView() const
		{
			return attachment_image_views[ current_image ];
		}
		static SwapChain create( VkInstance instance , VkPhysicalDevice pdev , VkDevice dev , uint images_count , OS::Window const &window )
		{
			VkWin32SurfaceCreateInfoKHR surface_create_info;
			Allocator::zero( &surface_create_info );
			surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_create_info.hinstance = window.hinstance;
			surface_create_info.hwnd = window.hwnd;
			SwapChain out;
			auto &surface = out.surface;
			surface.handle.create( instance , surface_create_info );
			vkGetPhysicalDeviceSurfaceFormatsKHR( pdev , *surface.handle , &surface.formats.size , NULL );
			vkGetPhysicalDeviceSurfaceFormatsKHR( pdev , *surface.handle , &surface.formats.size , &surface.formats[ 0 ] );
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR( pdev , *surface.handle , &surface.capabilities );
			vkGetPhysicalDeviceSurfacePresentModesKHR( pdev , *surface.handle , &surface.present_modes.size , nullptr );
			vkGetPhysicalDeviceSurfacePresentModesKHR( pdev , *surface.handle , &surface.present_modes.size , &surface.present_modes[ 0 ] );
			if( surface.formats[ 0 ].format == VK_FORMAT_UNDEFINED )
			{
				surface.preffered_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			} else
			{
				surface.preffered_format = surface.formats[ 0 ];
			}
			surface.preffered_format.colorSpace = surface.formats[ 0 ].colorSpace;
			surface.preffered_present_mode = VK_PRESENT_MODE_FIFO_KHR;
			for( auto &present_mode : surface.present_modes )
			{
				if( present_mode == VK_PRESENT_MODE_MAILBOX_KHR )
				{
					surface.preffered_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				} else if( present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR )
				{
					surface.preffered_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
			VkExtent2D swap_chain_extent;
			if( surface.capabilities.currentExtent.width == -1 )
			{
				swap_chain_extent.width = 512;
				swap_chain_extent.height = 512;
			} else
			{
				swap_chain_extent = surface.capabilities.currentExtent;
			}
			uint32_t desired_images_count = Math::MathUtil< uint >::min(
				Math::MathUtil< uint32_t >::max( surface.capabilities.minImageCount , images_count )
				, surface.capabilities.maxImageCount );
			VkSurfaceTransformFlagsKHR surface_transform_flags;
			if( surface.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
			{
				surface_transform_flags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			} else
			{
				surface_transform_flags = surface.capabilities.currentTransform;
			}
			VkSwapchainCreateInfoKHR swap_chain_create_info;
			Allocator::zero( &swap_chain_create_info );
			swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swap_chain_create_info.surface = *surface.handle;
			swap_chain_create_info.minImageCount = desired_images_count;
			swap_chain_create_info.imageFormat = surface.preffered_format.format;
			swap_chain_create_info.imageColorSpace = surface.preffered_format.colorSpace;
			swap_chain_create_info.imageExtent = swap_chain_extent;
			swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			swap_chain_create_info.preTransform = ( VkSurfaceTransformFlagBitsKHR )surface_transform_flags;
			swap_chain_create_info.imageArrayLayers = 1;
			swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swap_chain_create_info.presentMode = surface.preffered_present_mode;
			swap_chain_create_info.clipped = VK_TRUE;
			swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			out.handle.create( dev , swap_chain_create_info );
			uint image_count;
			vkGetSwapchainImagesKHR( dev , *out.handle , &image_count , nullptr );
			LocalArray< VkImage , 10 > images;
			vkGetSwapchainImagesKHR( dev , *out.handle , &image_count , &images[ 0 ] );
			out.dev_raw = dev;
			out.width = swap_chain_extent.width;
			out.height = swap_chain_extent.height;

			ito( image_count )
			{
				Image image;
				image.handle = images[ i ];
				image.format = swap_chain_create_info.imageFormat;
				image.layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				image.height = out.height;
				image.width = out.width;
				out.attachment_images.push( image );
				VkImageViewCreateInfo image_view_info;
				Allocator::zero( &image_view_info );
				image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_info.format = image.format;
				image_view_info.components = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
				image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				image_view_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 };
				image_view_info.image = image.handle;
				ImageView image_view;
				image_view.handle = Factory< VkImageView >::create( dev , image_view_info );
				image_view.range = image_view_info.subresourceRange;
				image_view.format = image.format;
				image_view.mapping = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
				out.attachment_image_views.push( image_view );
			}
			return out;
		}
	};
}