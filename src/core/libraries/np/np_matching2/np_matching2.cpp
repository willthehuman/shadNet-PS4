// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// NpMatching2 module (trace pass).
//
// The OpenOrbis NpMatching2.h header declares these as empty void stubs, so the
// real (C-linkage) signatures are declared here; the linker resolves the symbols
// from libSceNpMatching2. Every hook logs and forwards to the original function.

#include "core/libraries/np/np_matching2/np_matching2.h"

#include "common/logging/log.h"
#include "common/plugin_common.h"

extern "C" {
int sceNpMatching2CreateContext(const Libraries::Np::NpMatching2::OrbisNpMatching2CreateContextParameter* param, Libraries::Np::NpMatching2::OrbisNpMatching2ContextId* ctxId);
int sceNpMatching2CreateContextA(const Libraries::Np::NpMatching2::OrbisNpMatching2CreateContextParameterA* param, Libraries::Np::NpMatching2::OrbisNpMatching2ContextId* ctxId);
int sceNpMatching2CreateJoinRoom(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2CreateJoinRoomRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2JoinRoom(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2JoinRoomRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2CreateJoinRoomA(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2CreateJoinRoomRequestA* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2RegisterContextCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextCallback callback, void* userdata);
int sceNpMatching2RegisterLobbyEventCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2LobbyEventCallback callback, void* userdata);
int sceNpMatching2RegisterRoomEventCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomEventCallback callback, void* userdata);
int sceNpMatching2RegisterSignalingCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2SignalingCallback callback, void* userdata);
int sceNpMatching2ContextStart(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, u64 timeout);
int sceNpMatching2ContextStop(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId);
int sceNpMatching2Initialize(Libraries::Np::NpMatching2::OrbisNpMatching2InitializeParameter* param);
int sceNpMatching2Terminate();
int sceNpMatching2SetDefaultRequestOptParam(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt);
int sceNpMatching2GetServerId(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2ServerId* serverId);
int sceNpMatching2GetWorldInfoList(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetWorldInfoListRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2LeaveRoom(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2LeaveRoomRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SearchRoom(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2SearchRoomRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetUserInfo(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2SetUserInfoRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SendRoomMessage(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, void* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetRoomDataExternal(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2SetRoomDataExternalRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetRoomDataInternal(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2SetRoomDataInternalRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
s32 sceNpMatching2SignalingGetConnectionStatus(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomId roomId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomMemberId memberId, s32* connStatus, void* peerAddr, u16* peerPort);
int sceNpMatching2AbortContextStart(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId);
int sceNpMatching2DestroyContext(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId);
int sceNpMatching2RegisterLobbyMessageCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2LobbyMessageCallback callback, void* userdata);
int sceNpMatching2RegisterRoomMessageCallback(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomMessageCallback callback, void* userdata);
int sceNpMatching2GetMemoryInfo();
int sceNpMatching2GetSslMemoryInfo();
int sceNpMatching2GetRoomMemberIdListLocal();
int sceNpMatching2GetRoomPasswordLocal();
int sceNpMatching2GetSignalingOptParamLocal();
int sceNpMatching2GetLobbyInfoList();
int sceNpMatching2GetLobbyMemberDataInternal();
int sceNpMatching2GetLobbyMemberDataInternalList();
int sceNpMatching2GetRoomDataExternalList(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetRoomDataExternalListRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomDataInternal(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetRoomDataInternalRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomMemberDataExternalList(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetRoomMemberDataExternalListRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomMemberDataInternal();
int sceNpMatching2GetUserInfoListA(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetUserInfoListRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetUserInfoList(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2GetUserInfoListRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GrantRoomOwner(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2JoinLobby(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2JoinRoomA(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2JoinRoomRequestA* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2KickoutRoomMember(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2KickoutRoomMemberRequest* request, Libraries::Np::NpMatching2::OrbisNpMatching2RequestOptParam* requestOpt, Libraries::Np::NpMatching2::OrbisNpMatching2RequestId* requestId);
int sceNpMatching2LeaveLobby();
int sceNpMatching2SendLobbyChatMessage();
int sceNpMatching2SendRoomChatMessage(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2SetLobbyMemberDataInternal();
int sceNpMatching2SetRoomMemberDataInternal();
int sceNpMatching2SetSignalingOptParam();
int sceNpMatching2SignalingGetPeerNetInfo();
int sceNpMatching2SignalingGetPingInfo(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, const void* reqParam, const void* optParam, s32* reqId);
int sceNpMatching2SignalingCancelPeerNetInfo();
int sceNpMatching2SignalingGetConnectionInfoA(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomId roomId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo);
int sceNpMatching2SignalingGetConnectionInfo(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomId roomId, Libraries::Np::NpMatching2::OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo);
int sceNpMatching2SignalingGetLocalNetInfo(Libraries::Np::NpMatching2::OrbisNpMatching2ContextId ctxId, void* info);
int sceNpMatching2SignalingGetPeerNetInfoResult();
}

SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2CreateContext);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2CreateContextA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2CreateJoinRoom);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2JoinRoom);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2CreateJoinRoomA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterContextCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterLobbyEventCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterRoomEventCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterSignalingCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2ContextStart);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2ContextStop);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2Initialize);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2Terminate);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetDefaultRequestOptParam);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetServerId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetWorldInfoList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2LeaveRoom);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SearchRoom);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetUserInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SendRoomMessage);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetRoomDataExternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetRoomDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionStatus);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2AbortContextStart);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2DestroyContext);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterLobbyMessageCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2RegisterRoomMessageCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetMemoryInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetSslMemoryInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberIdListLocal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomPasswordLocal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetSignalingOptParamLocal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyInfoList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyMemberDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyMemberDataInternalList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomDataExternalList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberDataExternalList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetUserInfoListA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GetUserInfoList);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2GrantRoomOwner);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2JoinLobby);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2JoinRoomA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2KickoutRoomMember);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2LeaveLobby);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SendLobbyChatMessage);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SendRoomChatMessage);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetLobbyMemberDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetRoomMemberDataInternal);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SetSignalingOptParam);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPeerNetInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPingInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingCancelPeerNetInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionInfoA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetLocalNetInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPeerNetInfoResult);

namespace Libraries::Np::NpMatching2 {

int sceNpMatching2CreateContext(const OrbisNpMatching2CreateContextParameter* param, OrbisNpMatching2ContextId* ctxId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2CreateContext, param, ctxId);
    LOG_INFO(Lib_NpMatching2, "param={}; ctxId={}; ret={:#x}", fmt::ptr((const void*)(param)), fmt::ptr((const void*)(ctxId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2CreateContextA(const OrbisNpMatching2CreateContextParameterA* param, OrbisNpMatching2ContextId* ctxId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2CreateContextA, param, ctxId);
    LOG_INFO(Lib_NpMatching2, "param={}; ctxId={}; ret={:#x}", fmt::ptr((const void*)(param)), fmt::ptr((const void*)(ctxId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2CreateJoinRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2CreateJoinRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2CreateJoinRoom, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2JoinRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2JoinRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2JoinRoom, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2CreateJoinRoomA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2CreateJoinRoomRequestA* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2CreateJoinRoomA, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterContextCallback(OrbisNpMatching2ContextCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterContextCallback, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "callback={}; userdata={}; ret={:#x}", (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterLobbyEventCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LobbyEventCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterLobbyEventCallback, ctxId, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; callback={}; userdata={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterRoomEventCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomEventCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterRoomEventCallback, ctxId, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; callback={}; userdata={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterSignalingCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SignalingCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterSignalingCallback, ctxId, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; callback={}; userdata={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2ContextStart(OrbisNpMatching2ContextId ctxId, u64 timeout) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2ContextStart, ctxId, timeout);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; timeout={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(timeout), (unsigned)(ret));
    return ret;
}

int sceNpMatching2ContextStop(OrbisNpMatching2ContextId ctxId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2ContextStop, ctxId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned)(ret));
    return ret;
}

int sceNpMatching2Initialize(OrbisNpMatching2InitializeParameter* param) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2Initialize, param);
    LOG_INFO(Lib_NpMatching2, "param={}; ret={:#x}", fmt::ptr((const void*)(param)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2Terminate() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2Terminate>::wrap(Detour_sceNpMatching2Terminate);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetDefaultRequestOptParam(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RequestOptParam* requestOpt) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SetDefaultRequestOptParam, ctxId, requestOpt);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; requestOpt={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(requestOpt)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetServerId(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2ServerId* serverId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetServerId, ctxId, serverId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; serverId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(serverId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetWorldInfoList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetWorldInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetWorldInfoList, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2LeaveRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LeaveRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2LeaveRoom, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SearchRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SearchRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SearchRoom, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetUserInfo(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetUserInfoRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SetUserInfo, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SendRoomMessage(OrbisNpMatching2ContextId ctxId, void* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SendRoomMessage, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetRoomDataExternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetRoomDataExternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SetRoomDataExternal, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetRoomDataInternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetRoomDataInternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SetRoomDataInternal, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

s32 sceNpMatching2SignalingGetConnectionStatus(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, s32* connStatus, void* peerAddr, u16* peerPort) {
    const s32 ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SignalingGetConnectionStatus, ctxId, roomId, memberId, connStatus, peerAddr, peerPort);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; roomId={}; memberId={}; connStatus={}; peerAddr={}; peerPort={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(roomId), (unsigned long long)(memberId), fmt::ptr((const void*)(connStatus)), fmt::ptr((const void*)(peerAddr)), fmt::ptr((const void*)(peerPort)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2AbortContextStart(OrbisNpMatching2ContextId ctxId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2AbortContextStart, ctxId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned)(ret));
    return ret;
}

int sceNpMatching2DestroyContext(OrbisNpMatching2ContextId ctxId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2DestroyContext, ctxId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterLobbyMessageCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LobbyMessageCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterLobbyMessageCallback, ctxId, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; callback={}; userdata={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2RegisterRoomMessageCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomMessageCallback callback, void* userdata) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2RegisterRoomMessageCallback, ctxId, callback, userdata);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; callback={}; userdata={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(callback), fmt::ptr((const void*)(userdata)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetMemoryInfo() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetMemoryInfo>::wrap(Detour_sceNpMatching2GetMemoryInfo);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetSslMemoryInfo() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetSslMemoryInfo>::wrap(Detour_sceNpMatching2GetSslMemoryInfo);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomMemberIdListLocal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetRoomMemberIdListLocal>::wrap(Detour_sceNpMatching2GetRoomMemberIdListLocal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomPasswordLocal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetRoomPasswordLocal>::wrap(Detour_sceNpMatching2GetRoomPasswordLocal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetSignalingOptParamLocal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetSignalingOptParamLocal>::wrap(Detour_sceNpMatching2GetSignalingOptParamLocal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetLobbyInfoList() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetLobbyInfoList>::wrap(Detour_sceNpMatching2GetLobbyInfoList);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetLobbyMemberDataInternal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetLobbyMemberDataInternal>::wrap(Detour_sceNpMatching2GetLobbyMemberDataInternal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetLobbyMemberDataInternalList() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetLobbyMemberDataInternalList>::wrap(Detour_sceNpMatching2GetLobbyMemberDataInternalList);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomDataExternalList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomDataExternalListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetRoomDataExternalList, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomDataInternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomDataInternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetRoomDataInternal, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomMemberDataExternalList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomMemberDataExternalListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetRoomMemberDataExternalList, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetRoomMemberDataInternal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2GetRoomMemberDataInternal>::wrap(Detour_sceNpMatching2GetRoomMemberDataInternal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetUserInfoListA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetUserInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetUserInfoListA, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GetUserInfoList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetUserInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GetUserInfoList, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2GrantRoomOwner(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2GrantRoomOwner, ctxId, reqParam, optParam, reqId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; reqParam={}; optParam={}; reqId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(reqParam)), fmt::ptr((const void*)(optParam)), fmt::ptr((const void*)(reqId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2JoinLobby(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2JoinLobby, ctxId, reqParam, optParam, reqId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; reqParam={}; optParam={}; reqId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(reqParam)), fmt::ptr((const void*)(optParam)), fmt::ptr((const void*)(reqId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2JoinRoomA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2JoinRoomRequestA* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2JoinRoomA, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2KickoutRoomMember(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2KickoutRoomMemberRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2KickoutRoomMember, ctxId, request, requestOpt, requestId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; request={}; requestOpt={}; requestId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(request)), fmt::ptr((const void*)(requestOpt)), fmt::ptr((const void*)(requestId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2LeaveLobby() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2LeaveLobby>::wrap(Detour_sceNpMatching2LeaveLobby);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SendLobbyChatMessage() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SendLobbyChatMessage>::wrap(Detour_sceNpMatching2SendLobbyChatMessage);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SendRoomChatMessage(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SendRoomChatMessage, ctxId, reqParam, optParam, reqId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; reqParam={}; optParam={}; reqId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(reqParam)), fmt::ptr((const void*)(optParam)), fmt::ptr((const void*)(reqId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetLobbyMemberDataInternal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SetLobbyMemberDataInternal>::wrap(Detour_sceNpMatching2SetLobbyMemberDataInternal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetRoomMemberDataInternal() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SetRoomMemberDataInternal>::wrap(Detour_sceNpMatching2SetRoomMemberDataInternal);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SetSignalingOptParam() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SetSignalingOptParam>::wrap(Detour_sceNpMatching2SetSignalingOptParam);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetPeerNetInfo() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SignalingGetPeerNetInfo>::wrap(Detour_sceNpMatching2SignalingGetPeerNetInfo);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetPingInfo(OrbisNpMatching2ContextId ctxId, const void* reqParam, const void* optParam, s32* reqId) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SignalingGetPingInfo, ctxId, reqParam, optParam, reqId);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; reqParam={}; optParam={}; reqId={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(reqParam)), fmt::ptr((const void*)(optParam)), fmt::ptr((const void*)(reqId)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingCancelPeerNetInfo() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SignalingCancelPeerNetInfo>::wrap(Detour_sceNpMatching2SignalingCancelPeerNetInfo);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetConnectionInfoA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SignalingGetConnectionInfoA, ctxId, roomId, memberId, infoType, connInfo);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; roomId={}; memberId={}; infoType={}; connInfo={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(roomId), (unsigned long long)(memberId), (unsigned long long)(infoType), fmt::ptr((const void*)(connInfo)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetConnectionInfo(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SignalingGetConnectionInfo, ctxId, roomId, memberId, infoType, connInfo);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; roomId={}; memberId={}; infoType={}; connInfo={}; ret={:#x}", (unsigned long long)(ctxId), (unsigned long long)(roomId), (unsigned long long)(memberId), (unsigned long long)(infoType), fmt::ptr((const void*)(connInfo)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetLocalNetInfo(OrbisNpMatching2ContextId ctxId, void* info) {
    const int ret = SHADNET_HOOK_CONTINUE(sceNpMatching2SignalingGetLocalNetInfo, ctxId, info);
    LOG_INFO(Lib_NpMatching2, "ctxId={}; info={}; ret={:#x}", (unsigned long long)(ctxId), fmt::ptr((const void*)(info)), (unsigned)(ret));
    return ret;
}

int sceNpMatching2SignalingGetPeerNetInfoResult() {
    const int ret = HookContinueWrapperImpl<sceNpMatching2SignalingGetPeerNetInfoResult>::wrap(Detour_sceNpMatching2SignalingGetPeerNetInfoResult);
    LOG_INFO(Lib_NpMatching2, "ret={:#x}", (unsigned)(ret));
    return ret;
}

} // namespace Libraries::Np::NpMatching2

void RegisterNpMatching2Hooks() {
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2CreateContext);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2CreateContextA);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2CreateJoinRoom);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2JoinRoom);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2CreateJoinRoomA);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterContextCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterLobbyEventCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterRoomEventCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterSignalingCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2ContextStart);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2ContextStop);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2Initialize);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2Terminate);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetDefaultRequestOptParam);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetServerId);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetWorldInfoList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2LeaveRoom);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SearchRoom);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetUserInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SendRoomMessage);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetRoomDataExternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetRoomDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionStatus);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2AbortContextStart);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2DestroyContext);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterLobbyMessageCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2RegisterRoomMessageCallback);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetMemoryInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetSslMemoryInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberIdListLocal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomPasswordLocal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetSignalingOptParamLocal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyInfoList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyMemberDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetLobbyMemberDataInternalList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomDataExternalList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberDataExternalList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetRoomMemberDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetUserInfoListA);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GetUserInfoList);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2GrantRoomOwner);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2JoinLobby);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2JoinRoomA);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2KickoutRoomMember);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2LeaveLobby);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SendLobbyChatMessage);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SendRoomChatMessage);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetLobbyMemberDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetRoomMemberDataInternal);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SetSignalingOptParam);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPeerNetInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPingInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingCancelPeerNetInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionInfoA);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetConnectionInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetLocalNetInfo);
    SHADNET_HOOK(Libraries::Np::NpMatching2, sceNpMatching2SignalingGetPeerNetInfoResult);
}

namespace Libraries::Np::NpMatching2 {

void RegisterHooks() {
    return RegisterNpMatching2Hooks();
}

} // namespace Libraries::Np::NpMatching2
