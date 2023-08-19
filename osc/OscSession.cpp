/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "osc.h"
#include "OscPackage_generated.h"
#include "OscSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "Version.h"
#include "stdinput.h"
#include "stdoutput.h"
#include "string.h"
extern "C" {
#include "base64.h"
#include "lz4hc.h"
}
#include <assert.h>

//-----------------------------------------------------------------------------

OscSession::OscSession(nd::FiniteStateMachine* fsm)
	: nd::Session(fsm, 0)
    , inputTimerM(NULL)
    , peerVersionM(0)
{
	sendByeOnResetM = false;

}

//-----------------------------------------------------------------------------

OscSession::~OscSession()
{
    for(auto& kvp : zmodemFileMapM){
        delete kvp.second;
    }
    zmodemFileMapM.clear();

    stopInputTimer();
}

//-----------------------------------------------------------------------------

void OscSession::initState()
{
    for(auto& kvp : zmodemFileMapM){
        delete kvp.second;
    }
    zmodemFileMapM.clear();

	if (sendByeOnResetM){
        sendBye();

		if (!isToDelete()) asynHandleEvent(RESET_EVT);
	}
    bufferLenM = 0;
    memset(bufferM, 0, sizeof(bufferM));
	decodeIndexM = 0;
	sendByeOnResetM = false;
	//uploadFilePath_.clear();
	return;
}

//-----------------------------------------------------------------------------

void OscSession::parsePkg()
{
    while(bufferLenM > 0)
    {
        char* prefix_pos = nullptr; 
        while( bufferLenM > OSC_PREFIX_LEN ){
            prefix_pos = strstr(bufferM + decodeIndexM, OSC_PREFIX);
            if (prefix_pos == nullptr || prefix_pos > bufferM + bufferLenM - OSC_PREFIX_LEN){
                int advance_len = strlen(bufferM + decodeIndexM) + 1;
                if (advance_len > (int)bufferLenM - (int)OSC_PREFIX_LEN) {
                    advance_len = (int)bufferLenM - (int)OSC_PREFIX_LEN;
                }
                if (advance_len > 0){decodeIndexM += advance_len;}
                eatBuffer();
                continue;
            }

            break;
        }
        if (prefix_pos == nullptr || prefix_pos > bufferM + bufferLenM - OSC_PREFIX_LEN){
            return;
        }

        char* suffix_pos = strstr(prefix_pos + OSC_PREFIX_LEN, OSC_SUFFIX);
        if (suffix_pos == nullptr || suffix_pos >= bufferM + bufferLenM){
            return;
        }

        decodeIndexM = suffix_pos - bufferM;

        int base64_len = suffix_pos - prefix_pos - OSC_PREFIX_LEN;
        if (base64_len > OSC_MAX_ENCODED_PKG_SIZE){
            LOG_SE_ERROR("Error: package len:" << base64_len << " reach max size:" << OSC_MAX_ENCODED_PKG_SIZE << "!");
            asynHandleEvent(DESTROY_EVT);
            return;
        }
        uint8_t xz_buf[OSC_MAX_ENCODED_PKG_SIZE] = {0};
        int xz_len = base64_decode(prefix_pos + OSC_PREFIX_LEN, base64_len, xz_buf);
        if (xz_len <= 0){
            LOG_SE_ERROR("Error: base64 decode error:[" << std::string(prefix_pos + OSC_PREFIX_LEN, base64_len) << "]!");
            asynHandleEvent(DESTROY_EVT);
            return;
        }

        char decode_buf[OSC_MAX_ENCODED_PKG_SIZE] = {0};
        int decode_len = decodeLz(xz_buf, xz_len, (uint8_t*)decode_buf, sizeof(decode_buf));
        if (decode_len <= 0){
            LOG_SE_ERROR("Error: xz decode error!");
            asynHandleEvent(DESTROY_EVT);
            return;
        }

        // ok to adjust the recv buffer
        eatBuffer();

        auto pkg = nd::GetOscPkg(decode_buf);
        if (pkg->pkg_type() == nd::PkgType_InitRsp){
            peerVersionM = pkg->init_rsp()->version();
            asynHandleEvent(NEXT_EVT);
            LOG_SE_INFO("[handleInitRsp]version:" << (int)peerVersionM);
        }
        else if (pkg->pkg_type() == nd::PkgType_FileInfo){
        }
        else if (pkg->pkg_type() == nd::PkgType_FileProposeStart){
            handleFileProposeStart(pkg);
        }
        else if (pkg->pkg_type() == nd::PkgType_FileCompleteAck){
            handleFileCompleteAck(pkg);
        }
        else if (pkg->pkg_type() == nd::PkgType_Bye){
            LOG_SE_INFO("[handleBye]");
            sendByeBye();
        }
        else if (pkg->pkg_type() == nd::PkgType_ByeBye){
            LOG_SE_INFO("[handleByeBye]");
            asynHandleEvent(DESTROY_EVT);
        }
        else if (pkg->pkg_type() == nd::PkgType_Invalid){
            asynHandleEvent(DESTROY_EVT);
        }
    }

    return;
}

//-----------------------------------------------------------------------------

void OscSession::onInputTimerout(void *arg)
{
    OscSession* self = (OscSession*)arg;
    char buffer[4096] = {0};

    if (sizeof(self->bufferM) - self->bufferLenM < sizeof(buffer)){
        self->processNetworkInput(buffer, 0);
        self->inputTimerM = NULL;
        self->startInputTimer();
        return;
    }
    
    int len = 0;
    while((sizeof(self->bufferM) - self->bufferLenM > sizeof(buffer)) && (len = g_stdin->getInput(buffer, sizeof(buffer))) > 0)
    {
        self->processNetworkInput(buffer, len);
    }
    self->inputTimerM = NULL;
    self->startInputTimer();
}

void OscSession::startInputTimer()
{
    if (inputTimerM){stopInputTimer();}
    inputTimerM = g_processor->addLocalTimer(10, onInputTimerout, this);
}

//-----------------------------------------------------------------------------

void OscSession::stopInputTimer()
{
    if (!inputTimerM){return;}
    g_processor->cancelLocalTimer(inputTimerM);
}

//-----------------------------------------------------------------------------

int OscSession::processNetworkInput(const char* const str, const int len)
{	
    if (len > 0){
        memcpy(bufferM + bufferLenM, str, len);
        bufferLenM += len;
    }
	asynHandleEvent(NETWORK_INPUT_EVT);
	return 0;
}

//-----------------------------------------------------------------------------

void OscSession::reset()
{
	handleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void OscSession::output(const char* str, ...)
{
    char buffer[1024] = { 0 };
	va_list ap;
	va_start(ap, str);
	vsnprintf(buffer, sizeof(buffer), str, ap);
	va_end(ap);

    LOG_SE_DEBUG(buffer);
}

//-----------------------------------------------------------------------------

void OscSession::destroy()
{
    stopInputTimer();
	asynHandleEvent(DESTROY_EVT);
	setDelete();
}

//-----------------------------------------------------------------------------

void OscSession::deleteSelf(nd::Session* session)
{
    resetTty();
    delete session;
    g_processor->waitStop();
}

//-----------------------------------------------------------------------------

void OscSession::sendClientWorkingDir()
{
    auto clientWorkingDir = g_options->getClientWorkingDir();
    if(clientWorkingDir.empty()){return;}

    //sendZCommand(ZCMD_SET_CLIENT_WORKDIR, "%s", clientWorkingDir.c_str());
}

//-----------------------------------------------------------------------------

void OscSession::sendPkg(const uint8_t* fb_buf, int fb_len)
{
    uint8_t xz_buf[OSC_MAX_ENCODED_PKG_SIZE] = {0};
    int xz_len = encodeLz(fb_buf, fb_len, xz_buf, sizeof(xz_buf));
    if (xz_len == 0){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    char base64_buf[OSC_MAX_ENCODED_PKG_SIZE] = {0};
    if ((size_t)BASE64_CHARS(xz_len) >= sizeof(base64_buf)){
        LOG_SE_ERROR("[sendPkg]xz buff is too large! xz_len:" << xz_len)
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    int base64_len = base64_encode(xz_buf, xz_len, base64_buf);
    if (g_options->isInTmux()){
        g_stdout->sendData(TMUX_OSC_PREFIX, TMUX_OSC_PREFIX_LEN);
        g_stdout->sendData(base64_buf, base64_len);
        g_stdout->sendData(TMUX_OSC_SUFFIX, TMUX_OSC_SUFFIX_LEN);
    }else{
        g_stdout->sendData(OSC_PREFIX, OSC_PREFIX_LEN);
        g_stdout->sendData(base64_buf, base64_len);
        g_stdout->sendData(OSC_SUFFIX, OSC_SUFFIX_LEN);
    }
}

//-----------------------------------------------------------------------------

int OscSession::encodeLz(const uint8_t* fb_buf, int fb_len, uint8_t* buf, int len)
{
    return LZ4_compress_HC((const char*)fb_buf, (char*)buf, fb_len, len, LZ4HC_CLEVEL_MAX);
}

//-----------------------------------------------------------------------------

int OscSession::decodeLz(uint8_t* buf, int len, uint8_t* fb_buf, int fb_len)
{
    return LZ4_decompress_safe((const char*)buf, (char*)fb_buf, len, fb_len);
}

//-----------------------------------------------------------------------------

void OscSession::sendInitReq()
{
    flatbuffers::FlatBufferBuilder fbb;
    auto init_req = nd::CreateInitReq(fbb, OSC_VERSION);
    nd::OscPkgBuilder builder(fbb);
    builder.add_init_req(init_req);
    builder.add_pkg_type(nd::PkgType_InitReq);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    sendPkg(fb_buf, fb_len);
    LOG_SE_INFO("[sendInitReq]version:" << (int)OSC_VERSION) ;
}

//-----------------------------------------------------------------------------

void OscSession::sendFileInfo()
{
    auto zmodemFile = fetchNextFileToSend();
    if (zmodemFile == nullptr){
        asynHandleEvent(OK_EVT);
        return;
    }
    zmodemFileMapM[zmodemFile->getFileId()] = zmodemFile;

    flatbuffers::FlatBufferBuilder fbb;
    auto fb_file_info = nd::CreateFileInfoDirect(fbb, 
            zmodemFile->getFileId(), 
            zmodemFile->getFilename().c_str(), 
            zmodemFile->getFileSize());
    nd::OscPkgBuilder builder(fbb);
    builder.add_file_info(fb_file_info);
    builder.add_pkg_type(nd::PkgType_FileInfo);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    sendPkg(fb_buf, fb_len);

    LOG_SE_INFO("[sendFileInfo]id:" << zmodemFile->getFileId() 
            << ", file:" << zmodemFile->getFilename());
}

//-----------------------------------------------------------------------------

void OscSession::handleFileProposeStart(const nd::OscPkg *pkg)
{
    if (pkg->pkg_type() != nd::PkgType_FileProposeStart){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    auto file_propose_start = pkg->file_propose_start();
    LOG_SE_INFO("[handleFileProposeStart]id:" << file_propose_start->id()
            << ", offset:" << file_propose_start->exist_pos()
            << ", crc32:0x" << std::hex << file_propose_start->exist_crc32() << std::dec);

    auto it = zmodemFileMapM.find(file_propose_start->id());
    if (it == zmodemFileMapM.end()){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    auto zmodemFile = it->second;
    uint64_t validLen = zmodemFile->validateFileCrc(file_propose_start->exist_pos(), file_propose_start->exist_crc32());
    if (validLen == zmodemFile->getFileSize()){
        sendFileComplete(zmodemFile);
        return;
    }
    zmodemFile->setReadPos(validLen);
    asynHandleEvent(SEND_DATA_EVT);

    sendFileInitPos(zmodemFile);
    return;

}

//-----------------------------------------------------------------------------

void OscSession::sendFileInitPos(ZmodemFile* zmodemFile)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto fb_file_init_pos = nd::CreateFileInitPos(fbb, 
            zmodemFile->getFileId(), 
            zmodemFile->getPos());

    nd::OscPkgBuilder builder(fbb);
    builder.add_file_init_pos(fb_file_init_pos);
    builder.add_pkg_type(nd::PkgType_FileInitPos);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    sendPkg(fb_buf, fb_len);

    LOG_SE_INFO("[sendFileInitPos]id:" << zmodemFile->getFileId() 
            << ", offset:" << zmodemFile->getPos()
            << ", file:" << zmodemFile->getFilename());
}

//-----------------------------------------------------------------------------

void OscSession::sendFileContent()
{
    if (zmodemFileMapM.empty()){
        asynHandleEvent(NEXT_EVT);
        return;
    }

    for(auto& it : zmodemFileMapM)
    {
        auto zmodemFile = it.second;
        if (!zmodemFile->isGood()){
            continue;
        }

        char buffer[OSC_MAX_RAW_CONTENT_SIZE] = {0};
        uint64_t offset = zmodemFile->getPos();
        unsigned len = zmodemFile->read(buffer, OSC_MAX_RAW_CONTENT_SIZE);
        if (len == 0){
            continue;
        }

        uint32_t crc32 = calcBufferCrc32(buffer, len);
        {
            flatbuffers::FlatBufferBuilder fbb;
            auto content = fbb.CreateString(buffer, len);

            auto fb_file_content = nd::CreateFileContent(fbb, 
                    zmodemFile->getFileId(), 
                    offset, // offset
                    crc32, //
                    content);

            nd::OscPkgBuilder builder(fbb);
            builder.add_file_content(fb_file_content);
            builder.add_pkg_type(nd::PkgType_FileContent);
            fbb.Finish(builder.Finish());

            auto fb_buf = fbb.GetBufferPointer();
            auto fb_len = fbb.GetSize();

            sendPkg(fb_buf, fb_len);
            LOG_SE_INFO("[sendFileContent]id:" << zmodemFile->getFileId() 
                    << ", offset:" << offset
                    << ", len:" << len
                    << ", crc32:0x" << std::hex << crc32 << std::dec
                    << ", file:" << zmodemFile->getFilename());
        }

        if(!zmodemFile->isGood()){
            sendFileComplete(zmodemFile);
            return;
        }else{
            asynHandleEvent(SEND_DATA_EVT);
            return;
        }
    }

    asynHandleEvent(NEXT_EVT);
    return;

}

//-----------------------------------------------------------------------------

void OscSession::sendFileComplete(ZmodemFile* zmodemFile)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto fb_file_complete = nd::CreateFileComplete(fbb, 
            zmodemFile->getFileId(), 
            zmodemFile->getFileSize());

    nd::OscPkgBuilder builder(fbb);
    builder.add_file_complete(fb_file_complete);
    builder.add_pkg_type(nd::PkgType_FileComplete);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    LOG_SE_INFO("[sendFileComplete]id:" << zmodemFile->getFileId() 
            << ", size:" << zmodemFile->getFileSize() 
            << ", file:" << zmodemFile->getFilename());
    sendPkg(fb_buf, fb_len);
}

//-----------------------------------------------------------------------------

void OscSession::handleFileCompleteAck(const nd::OscPkg *pkg)
{
    if (pkg->pkg_type() != nd::PkgType_FileCompleteAck){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    auto file_complete_ack = pkg->file_complete_ack();
    LOG_SE_INFO("[handleFileCompleteAck]id:" << file_complete_ack->id());

    auto it = zmodemFileMapM.find(file_complete_ack->id());
    if (it == zmodemFileMapM.end()){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    delete it->second;
    zmodemFileMapM.erase(it);
    asynHandleEvent(NEXT_EVT);
    return;

}

//-----------------------------------------------------------------------------

void OscSession::sendBye()
{
    flatbuffers::FlatBufferBuilder fbb;
    auto bye = nd::CreateBye(fbb);
    nd::OscPkgBuilder builder(fbb);
    builder.add_bye(bye);
    builder.add_pkg_type(nd::PkgType_Bye);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    sendPkg(fb_buf, fb_len);
    LOG_SE_INFO("[sendBye]");
}

//-----------------------------------------------------------------------------

void OscSession::sendByeBye()
{
    flatbuffers::FlatBufferBuilder fbb;
    auto bye = nd::CreateBye(fbb);
    nd::OscPkgBuilder builder(fbb);
    builder.add_bye(bye);
    builder.add_pkg_type(nd::PkgType_Bye);
    fbb.Finish(builder.Finish());

    auto fb_buf = fbb.GetBufferPointer();
    auto fb_len = fbb.GetSize();

    sendPkg(fb_buf, fb_len);

    asynHandleEvent(RESET_EVT);
    LOG_SE_INFO("[sendByeBye]");
}


//-----------------------------------------------------------------------------

