// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// NpMatching2 module (trace pass) — ported from the bloodborne_pc/shadPS4
// reference implementation. Hooks currently log and forward to the original
// Sony implementations; real shadNet-backed implementations replace the
// forward calls incrementally.

#pragma once

#include "common/types.h"
#include "core/libraries/np/np_types2.h"

namespace Libraries::Np::NpMatching2 {

int sceNpMatching2CreateContext(const OrbisNpMatching2CreateContextParameter* param, OrbisNpMatching2ContextId* ctxId);
int sceNpMatching2CreateContextA(const OrbisNpMatching2CreateContextParameterA* param, OrbisNpMatching2ContextId* ctxId);
int sceNpMatching2CreateJoinRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2CreateJoinRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2JoinRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2JoinRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2CreateJoinRoomA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2CreateJoinRoomRequestA* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2RegisterContextCallback(OrbisNpMatching2ContextCallback callback, void* userdata);
int sceNpMatching2RegisterLobbyEventCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LobbyEventCallback callback, void* userdata);
int sceNpMatching2RegisterRoomEventCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomEventCallback callback, void* userdata);
int sceNpMatching2RegisterSignalingCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SignalingCallback callback, void* userdata);
int sceNpMatching2ContextStart(OrbisNpMatching2ContextId ctxId, u64 timeout);
int sceNpMatching2ContextStop(OrbisNpMatching2ContextId ctxId);
int sceNpMatching2Initialize(OrbisNpMatching2InitializeParameter* param);
int sceNpMatching2Terminate();
int sceNpMatching2SetDefaultRequestOptParam(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RequestOptParam* requestOpt);
int sceNpMatching2GetServerId(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2ServerId* serverId);
int sceNpMatching2GetWorldInfoList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetWorldInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2LeaveRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LeaveRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SearchRoom(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SearchRoomRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetUserInfo(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetUserInfoRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SendRoomMessage(OrbisNpMatching2ContextId ctxId, void* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetRoomDataExternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetRoomDataExternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2SetRoomDataInternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2SetRoomDataInternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
s32 sceNpMatching2SignalingGetConnectionStatus(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, s32* connStatus, void* peerAddr, u16* peerPort);
int sceNpMatching2AbortContextStart(OrbisNpMatching2ContextId ctxId);
int sceNpMatching2DestroyContext(OrbisNpMatching2ContextId ctxId);
int sceNpMatching2RegisterLobbyMessageCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2LobbyMessageCallback callback, void* userdata);
int sceNpMatching2RegisterRoomMessageCallback(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomMessageCallback callback, void* userdata);
int sceNpMatching2GetMemoryInfo();
int sceNpMatching2GetSslMemoryInfo();
int sceNpMatching2GetRoomMemberIdListLocal();
int sceNpMatching2GetRoomPasswordLocal();
int sceNpMatching2GetSignalingOptParamLocal();
int sceNpMatching2GetLobbyInfoList();
int sceNpMatching2GetLobbyMemberDataInternal();
int sceNpMatching2GetLobbyMemberDataInternalList();
int sceNpMatching2GetRoomDataExternalList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomDataExternalListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomDataInternal(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomDataInternalRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomMemberDataExternalList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetRoomMemberDataExternalListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetRoomMemberDataInternal();
int sceNpMatching2GetUserInfoListA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetUserInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GetUserInfoList(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2GetUserInfoListRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2GrantRoomOwner(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2JoinLobby(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2JoinRoomA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2JoinRoomRequestA* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2KickoutRoomMember(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2KickoutRoomMemberRequest* request, OrbisNpMatching2RequestOptParam* requestOpt, OrbisNpMatching2RequestId* requestId);
int sceNpMatching2LeaveLobby();
int sceNpMatching2SendLobbyChatMessage();
int sceNpMatching2SendRoomChatMessage(OrbisNpMatching2ContextId ctxId, void* reqParam, void* optParam, s32* reqId);
int sceNpMatching2SetLobbyMemberDataInternal();
int sceNpMatching2SetRoomMemberDataInternal();
int sceNpMatching2SetSignalingOptParam();
int sceNpMatching2SignalingGetPeerNetInfo();
int sceNpMatching2SignalingGetPingInfo(OrbisNpMatching2ContextId ctxId, const void* reqParam, const void* optParam, s32* reqId);
int sceNpMatching2SignalingCancelPeerNetInfo();
int sceNpMatching2SignalingGetConnectionInfoA(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo);
int sceNpMatching2SignalingGetConnectionInfo(OrbisNpMatching2ContextId ctxId, OrbisNpMatching2RoomId roomId, OrbisNpMatching2RoomMemberId memberId, u32 infoType, void* connInfo);
int sceNpMatching2SignalingGetLocalNetInfo(OrbisNpMatching2ContextId ctxId, void* info);
int sceNpMatching2SignalingGetPeerNetInfoResult();

void RegisterHooks();

} // namespace Libraries::Np::NpMatching2
