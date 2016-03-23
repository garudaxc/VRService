#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/fb.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <hardware/hwcomposer_defs.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <ui/Fence.h>
#include <utils/BitSet.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/Thread.h>
#include <utils/Timers.h>
#include <utils/Vector.h>
#include <cutils/properties.h>
#include <android/configuration.h>

using namespace android;


#undef ALOGI
#undef ALOGW
#undef ALOGE


void MyLog(const char* str, ...)
{
    char buffer[256];
    sprintf(buffer, "%s\n", str);
    va_list argptr;
    va_start(argptr, str);

    vprintf(buffer, argptr);
    va_end(argptr);
}

#define ALOGI MyLog
#define ALOGW MyLog
#define ALOGE MyLog




inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

static void log_fscreeninfo(const struct fb_fix_screeninfo* finfo)
{
	printf("fixscreen info:\n");

	printf("id %s\n",finfo->id);       //s3c2410fb
	printf("smem_start %u\n",(unsigned int)finfo->smem_start); //帧缓冲 内存开始地址,物理地址
	printf("smem_len %d\n",finfo->smem_len); // 帧缓冲 内存 长度
	printf("type %d\n",finfo->type);
	printf("type_aux %d\n",finfo->type_aux);//平面交织交替
	printf("visual %d\n",finfo->visual); //记录 色彩模式   2
	printf("xpanstep %d\n",finfo->xpanstep);//如果没有硬件panning，赋0
	printf("ypanstep %d\n",finfo->ypanstep);
	printf("line_length %d\n",finfo->line_length);  //  640
	printf("mmio_start %u\n",(unsigned int)finfo->mmio_start);//内存映射IO开始地址 物理地址
	printf("mmio_len %d\n",finfo->mmio_len);//内存映射IO 长度
	printf("accel %d\n\n",finfo->accel);
}

static void log_vscreeninfo(const struct fb_var_screeninfo* vinfo)
{
    printf("log_vscreeninfo info:\n");
	printf("xres %d\n",vinfo->xres); //可见解析度  320
    printf("yres %d\n",vinfo->yres);          //  240
    printf("xres_virturl %d\n",vinfo->xres_virtual); //虚拟解析度  320
    printf("yres_virtual %d\n",vinfo->yres_virtual);           //   240
    printf("xoffset %d\n",vinfo->xoffset); //虚拟到可见的偏移        0
    printf("yoffset %d\n",vinfo->yoffset);                     //    0
    printf("bits_per_pixel %d\n",vinfo->bits_per_pixel); //每像素位数 bpp  16
    printf("grayscale %d\n",vinfo->grayscale);//非零时，指灰度

    printf("fb_bitfield red.offset %d\n",vinfo->red.offset);     //11  偏移11位
    printf("fb_bitfield .length %d\n",vinfo->red.length);        // 5
    printf("fb_bitfield .msb_right %d\n",vinfo->red.msb_right);  //  0
    printf("fb_bitfield green.offset %d\n",vinfo->green.offset); // 5 偏移5位
    printf("fb_bitfield .length %d\n",vinfo->green.length);       // 6
    printf("fb_bitfield .msb_right %d\n",vinfo->green.msb_right); // 0
    printf("fb_bitfield blue.offset %d\n",vinfo->blue.offset);
    printf("fb_bitfield .length %d\n",vinfo->blue.length);
    printf("fb_bitfield .msb_right %d\n",vinfo->blue.msb_right);
    printf("fb_bitfield transp.offset %d\n",vinfo->transp.offset);
    printf("fb_bitfield .length %d\n",vinfo->transp.length);
    printf("fb_bitfield .msb_right %d\n",vinfo->transp.msb_right);

    printf("nonstd %d\n",vinfo->nonstd); //!=0 非标准像素格式
    printf("activate %d\n",vinfo->activate);
    printf("height %d\n",vinfo->height); //高度/  240
    printf("widht %d\n",vinfo->width);          //   320
    printf("accel_flags %d\n",vinfo->accel_flags); //看 fb_info.flags

    //定时，除了 pixclock之外，其他的都以像素时钟为单位
    printf("pixclock %d\n",vinfo->pixclock);//像素时钟，皮秒   80000
    printf("left_margin %d\n",vinfo->left_margin);//行切换：从同步到绘图之间的延迟    28
    printf("right_margin %d\n",vinfo->right_margin);//行切换：从绘图到同步之间的延迟   24
    printf("upper_margin %d\n",vinfo->upper_margin);//帧切换：从同步到绘图之间的延迟   6
    printf("lower_margin %d\n",vinfo->lower_margin);//帧切换：从绘图到同步之间的延迟    2

    printf("hsync_len %d\n",vinfo->hsync_len); //hor 水平同步的长度         42
    printf("vsync_len %d\n",vinfo->vsync_len); //vir 垂直同步的长度         12

    printf("sync %d\n",vinfo->sync); //
    printf("vmode %d\n",vinfo->vmode);
    printf("rotate %d\n",vinfo->rotate);
}


#define MDP_DISPLAY_COMMIT_OVERLAY 0x00000001

struct mdp_display_commit {
    uint32_t flags;
    uint32_t wait_for_finish;
    struct fb_var_screeninfo var;
};

#define MSMFB_IOCTL_MAGIC 'm'
#define MSMFB_DISPLAY_COMMIT _IOW(MSMFB_IOCTL_MAGIC, 164, struct mdp_display_commit)


int FramebufferTest()
{
    char const * const device_template[] = {
            "/dev/graphics/fb%u",
            "/dev/fb%u",
            0 };

    int fd = -1;
    int i=0;
    char name[64];
    char property[PROPERTY_VALUE_MAX];

    while ((fd==-1) && device_template[i]) {
        snprintf(name, 64, device_template[i], 0);
        fd = open(name, O_RDWR, 0);
        i++;
    }
    if (fd < 0){
        printf("can not open devcies fb\n");
        return -errno;
    }

//    memset(&module->commit, 0, sizeof(struct mdp_display_commit));
//
    struct fb_fix_screeninfo finfo;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1){
        printf("can not FBIOGET_FSCREENINFO %d\n", errno);
        return -errno;
    }
    log_fscreeninfo(&finfo);

    struct fb_var_screeninfo info;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1){
        printf("can not FBIOGET_VSCREENINFO %d\n", errno);
        return -errno;
    }

    log_vscreeninfo(&info);

    int fbSize = 1920 * 1080 * 4 + 1920 * 200 * 4;
    fbSize = roundUpToPageSize(fbSize);
    void* vaddr = mmap(0, fbSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (vaddr == NULL) {
        printf("mmap failed!\n");
        return -1;
    }

    memset(vaddr, 0, fbSize);
    printf("map addr %p\n", vaddr);

    return 0;
}


#define MIN_HWC_HEADER_VERSION HWC_HEADER_VERSION


static uint32_t hwcApiVersion(const hwc_composer_device_1_t* hwc) {
    uint32_t hwcVersion = hwc->common.version;
    return hwcVersion & HARDWARE_API_VERSION_2_MAJ_MIN_MASK;
}

static uint32_t hwcHeaderVersion(const hwc_composer_device_1_t* hwc) {
    uint32_t hwcVersion = hwc->common.version;
    return hwcVersion & HARDWARE_API_VERSION_2_HEADER_MASK;
}

static bool hwcHasApiVersion(const hwc_composer_device_1_t* hwc,
                             uint32_t version) {
    return hwcApiVersion(hwc) >= (version & HARDWARE_API_VERSION_2_MAJ_MIN_MASK);
}

framebuffer_device_t*           mFbDev;
struct hwc_composer_device_1*   mHwc;


int loadFbHalModule()
{
    hw_module_t const* module;

    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (err != 0) {
        ALOGE("%s module not found", GRALLOC_HARDWARE_MODULE_ID);
        return err;
    }

    return framebuffer_open(module, &mFbDev);
}


// Load and prepare the hardware composer module.  Sets mHwc.
void loadHwcModule()
{
    hw_module_t const* module;

    if (hw_get_module(HWC_HARDWARE_MODULE_ID, &module) != 0) {
        ALOGE("%s module not found", HWC_HARDWARE_MODULE_ID);
        return;
    }

    int err = hwc_open_1(module, &mHwc);
    if (err) {
        ALOGE("%s device failed to initialize (%s)",
              HWC_HARDWARE_COMPOSER, strerror(-err));
        return;
    }

    if (!hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_0) ||
        hwcHeaderVersion(mHwc) < MIN_HWC_HEADER_VERSION ||
        hwcHeaderVersion(mHwc) > HWC_HEADER_VERSION) {
        ALOGE("%s device version %#x unsupported, will not be used",
              HWC_HARDWARE_COMPOSER, mHwc->common.version);
        hwc_close_1(mHwc);
        mHwc = NULL;
        return;
    }
}


    struct DisplayData {
        DisplayData();
        ~DisplayData();
        uint32_t width;
        uint32_t height;
        uint32_t format;    // pixel format from FB hal, for pre-hwc-1.1
        float xdpi;
        float ydpi;
        nsecs_t refresh;
        bool connected;
        bool hasFbComp;
        bool hasOvComp;
        size_t capacity;
        hwc_display_contents_1* list;
        hwc_layer_1* framebufferTarget;
        buffer_handle_t fbTargetHandle;
        sp<Fence> lastRetireFence;  // signals when the last set op retires
        sp<Fence> lastDisplayFence; // signals when the last set op takes
                                    // effect on screen
        buffer_handle_t outbufHandle;
        sp<Fence> outbufAcquireFence;

        // protected by mEventControlLock
        int32_t events;
    };
DisplayData::DisplayData()
:   width(0), height(0), format(0),
    xdpi(0.0f), ydpi(0.0f),
    refresh(0),
    connected(false),
    hasFbComp(false), hasOvComp(false),
    capacity(0), list(NULL),
    framebufferTarget(NULL), fbTargetHandle(0),
    lastRetireFence(Fence::NO_FENCE), lastDisplayFence(Fence::NO_FENCE),
    outbufHandle(NULL), outbufAcquireFence(Fence::NO_FENCE),
    events(0)
{}

DisplayData::~DisplayData() {
    free(list);
}



static float getDefaultDensity(uint32_t height) {
    if (height >= 1080) return ACONFIGURATION_DENSITY_XHIGH;
    else                return ACONFIGURATION_DENSITY_TV;
}

static const uint32_t DISPLAY_ATTRIBUTES[] = {
    HWC_DISPLAY_VSYNC_PERIOD,
    HWC_DISPLAY_WIDTH,
    HWC_DISPLAY_HEIGHT,
    HWC_DISPLAY_DPI_X,
    HWC_DISPLAY_DPI_Y,
    HWC_DISPLAY_NO_ATTRIBUTE,
};

#define NUM_DISPLAY_ATTRIBUTES (sizeof(DISPLAY_ATTRIBUTES) / sizeof(DISPLAY_ATTRIBUTES)[0])

DisplayData       mDisplayData[2];

int queryDisplayProperties(int disp) {

    // use zero as default value for unspecified attributes
    int32_t values[NUM_DISPLAY_ATTRIBUTES - 1];
    memset(values, 0, sizeof(values));

    uint32_t config;
    size_t numConfigs = 1;
    status_t err = mHwc->getDisplayConfigs(mHwc, disp, &config, &numConfigs);
    if (err != NO_ERROR) {
        // this can happen if an unpluggable display is not connected
        mDisplayData[disp].connected = false;
        return err;
    }

    err = mHwc->getDisplayAttributes(mHwc, disp, config, DISPLAY_ATTRIBUTES, values);
    if (err != NO_ERROR) {
        // we can't get this display's info. turn it off.
        mDisplayData[disp].connected = false;
        return err;
    }

    int32_t w = 0, h = 0;
    for (size_t i = 0; i < NUM_DISPLAY_ATTRIBUTES - 1; i++) {
        switch (DISPLAY_ATTRIBUTES[i]) {
        case HWC_DISPLAY_VSYNC_PERIOD:
            mDisplayData[disp].refresh = nsecs_t(values[i]);
            break;
        case HWC_DISPLAY_WIDTH:
            mDisplayData[disp].width = values[i];
            break;
        case HWC_DISPLAY_HEIGHT:
            mDisplayData[disp].height = values[i];
            break;
        case HWC_DISPLAY_DPI_X:
            mDisplayData[disp].xdpi = values[i] / 1000.0f;
            break;
        case HWC_DISPLAY_DPI_Y:
            mDisplayData[disp].ydpi = values[i] / 1000.0f;
            break;
        default:
            ALOG_ASSERT(false, "unknown display attribute[%d] %#x",
                    i, DISPLAY_ATTRIBUTES[i]);
            break;
        }
    }

    // FIXME: what should we set the format to?
    mDisplayData[disp].format = HAL_PIXEL_FORMAT_RGBA_8888;
    mDisplayData[disp].connected = true;
    if (mDisplayData[disp].xdpi == 0.0f || mDisplayData[disp].ydpi == 0.0f) {
        float dpi = getDefaultDensity(h);
        mDisplayData[disp].xdpi = dpi;
        mDisplayData[disp].ydpi = dpi;
    }
    return NO_ERROR;
}



status_t createWorkList(int32_t id, size_t numLayers) {
    ALOGI("createWorkList");

    if (hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_1)) {
        ALOGI("HWC_DEVICE_API_VERSION_1_1 true");
    }

    if (hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_3)) {
        ALOGI("HWC_DEVICE_API_VERSION_1_3 true");
    }

    if (mHwc) {
        DisplayData& disp(mDisplayData[id]);
        if (hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_1)) {
            // we need space for the HWC_FRAMEBUFFER_TARGET
            numLayers++;
        }
        if (disp.capacity < numLayers || disp.list == NULL) {
            size_t size = sizeof(hwc_display_contents_1_t)
                    + numLayers * sizeof(hwc_layer_1_t);
            free(disp.list);
            disp.list = (hwc_display_contents_1_t*)malloc(size);
            disp.capacity = numLayers;
        }
        if (hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_1)) {
            disp.framebufferTarget = &disp.list->hwLayers[numLayers - 1];
            memset(disp.framebufferTarget, 0, sizeof(hwc_layer_1_t));
            const hwc_rect_t r = { 0, 0, (int) disp.width, (int) disp.height };
            disp.framebufferTarget->compositionType = HWC_FRAMEBUFFER_TARGET;
            disp.framebufferTarget->hints = 0;
            disp.framebufferTarget->flags = 0;
            disp.framebufferTarget->handle = disp.fbTargetHandle;
            disp.framebufferTarget->transform = 0;
            disp.framebufferTarget->blending = HWC_BLENDING_PREMULT;
            if (hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_3)) {
                disp.framebufferTarget->sourceCropf.left = 0;
                disp.framebufferTarget->sourceCropf.top = 0;
                disp.framebufferTarget->sourceCropf.right = disp.width;
                disp.framebufferTarget->sourceCropf.bottom = disp.height;
            } else {
                disp.framebufferTarget->sourceCrop = r;
            }
            disp.framebufferTarget->displayFrame = r;
            disp.framebufferTarget->visibleRegionScreen.numRects = 1;
            disp.framebufferTarget->visibleRegionScreen.rects =
                &disp.framebufferTarget->displayFrame;
            disp.framebufferTarget->acquireFenceFd = -1;
            disp.framebufferTarget->releaseFenceFd = -1;
            disp.framebufferTarget->planeAlpha = 0xFF;
        }
        disp.list->retireFenceFd = -1;
        disp.list->flags = HWC_GEOMETRY_CHANGED;
        disp.list->numHwLayers = numLayers;
    }
    return NO_ERROR;
}



int main(int argc, char **argv) {
    ALOGI("fbtest !!!!!!");

    loadFbHalModule();
    loadHwcModule();

    ALOGI("mFbDev %p mHwc %p", mFbDev, mHwc);

    if (queryDisplayProperties(0) != NO_ERROR) {
        ALOGE("queryDisplayProperties 0 failed!");
    }
    ALOGI("display 0 info:");
    ALOGI("width %d height %d", mDisplayData[0].width, mDisplayData[0].height);

    createWorkList(0, 1);



//    info.activate = FB_ACTIVATE_VBL;
//    info.yoffset = 0;
//    if (ioctl(fd, FBIOPUT_VSCREENINFO, &info) == -1) {
//        printf("%s: FBIOPUT_VSCREENINFO for primary failed, str: %s\n",
//              __FUNCTION__, strerror(errno));
//        return -errno;
//    }
//    printf("FBIOPUT_VSCREENINFO\n");

//    i = 50;
//    while (i--) {
//
//        struct mdp_display_commit commit_info;
//        memset(&commit_info, 0, sizeof(struct mdp_display_commit));
//        commit_info.flags = MDP_DISPLAY_COMMIT_OVERLAY;
//        if(ioctl(fd, MSMFB_DISPLAY_COMMIT, &commit_info) == -1) {
//            printf("%s: MSMFB_DISPLAY_COMMIT for primary failed\n", __FUNCTION__);
//            return -errno;
//        }
//
//        printf("MSMFB_DISPLAY_COMMIT\n");
//    }


    return 0;
}