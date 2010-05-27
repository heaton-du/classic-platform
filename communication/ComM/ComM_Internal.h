/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#ifndef COMM_INTERNAL_H_
#define COMM_INTERNAL_H_

#include "ComM_Types.h"

#if (COMM_DEV_ERROR_DETECT == STD_ON)
#define COMM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_COMM, 0, serviceId, errorId)

#define COMM_VALIDATE(expression, serviceId, errorId, ...)	\
	if (!(expression)) {									\
		COMM_DET_REPORTERROR(serviceId, errorId);			\
		return __VA_ARGS__;									\
	}

#else
#define COMM_DET_REPORTERROR(...)
#define COMM_VALIDATE(...)
#endif

#define COMM_VALIDATE_INIT(serviceID, ...)					\
		COMM_VALIDATE((ComM_Internal.InitStatus == COMM_INIT), serviceID, COMM_E_NOT_INITED, __VA_ARGS__)

#define COMM_VALIDATE_PARAMETER(expression, serviceID, ...)					\
		COMM_VALIDATE(expression, serviceID, COMM_E_WRONG_PARAMETERS, __VA_ARGS__)

#define COMM_VALIDATE_CHANNEL(channel, serviceID, ...)					\
		COMM_VALIDATE_PARAMETER( (channel < COMM_CHANNEL_COUNT), serviceID, __VA_ARGS__)

#define COMM_VALIDATE_USER(user, serviceID, ...)					\
		COMM_VALIDATE_PARAMETER( (user < COMM_USER_COUNT), serviceID, __VA_ARGS__)


typedef enum {
	COMM_SUBMODE_NETWORK_REQUESTED,
	COMM_SUBMODE_READY_SLEEP,
	COMM_SUBMODE_NONE,
} ComM_Internal_SubModeType;

typedef struct {
	ComM_ModeType				Mode;
	ComM_Internal_SubModeType	SubMode;
	uint32						UserRequestMask;
	ComM_InhibitionStatusType	InhibitionStatus;
} ComM_Internal_ChannelType;

typedef struct {
	ComM_ModeType				RequestedMode;
} ComM_Internal_UserType;

typedef struct {
	ComM_InitStatusType 		InitStatus;
	ComM_Internal_ChannelType	Channels[COMM_CHANNEL_COUNT];
	ComM_Internal_UserType		Users[COMM_USER_COUNT];
	boolean 					NoCommunication;
	uint16						InhibitCounter;
} ComM_InternalType;

/* Delegate request to users channels and call ComM_Internal_UpdateChannelState */
static Std_ReturnType ComM_Internal_RequestComMode(
				ComM_UserHandleType User, ComM_ModeType ComMode );

/* Looks at stored requests for Channel and updates state accordingly */
static Std_ReturnType ComM_Internal_UpdateChannelState( const ComM_ChannelType* Channel, boolean isRequest );

/* Propagates channel mode to respective Bus SM */
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* Channel );

/* Propagate query to channel Bus SMs. Collect overall mode and status */
static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode(
		ComM_UserHandleType User, ComM_ModeType* ComMode );

#endif /* COMM_INTERNAL_H_ */
