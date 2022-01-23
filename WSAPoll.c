//WSAIoctl handler the same for
//SIO_BASE_HANDLE
//SIO_BSP_HANDLE
//SIO_BSP_HANDLE_SELECT
//SIO_BSP_HANDLE_POLL
//SOCKET_ERROR / WSAEFAULT

//
// Structures for IOCTL_AFD_POLL.
//

typedef struct _AFD_POLL_HANDLE_INFO {
    HANDLE Handle;
    ULONG PollEvents;
    NTSTATUS Status;
} AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

typedef struct _AFD_POLL_INFO {
    LARGE_INTEGER Timeout;
    ULONG NumberOfHandles;
    BOOLEAN Unique;
    AFD_POLL_HANDLE_INFO Handles[1];
} AFD_POLL_INFO, *PAFD_POLL_INFO;

#define AFD_POLL_RECEIVE_BIT            0
#define AFD_POLL_RECEIVE                (1 << AFD_POLL_RECEIVE_BIT)
#define AFD_POLL_RECEIVE_EXPEDITED_BIT  1
#define AFD_POLL_RECEIVE_EXPEDITED      (1 << AFD_POLL_RECEIVE_EXPEDITED_BIT)
#define AFD_POLL_SEND_BIT               2
#define AFD_POLL_SEND                   (1 << AFD_POLL_SEND_BIT)
#define AFD_POLL_DISCONNECT_BIT         3
#define AFD_POLL_DISCONNECT             (1 << AFD_POLL_DISCONNECT_BIT)
#define AFD_POLL_ABORT_BIT              4
#define AFD_POLL_ABORT                  (1 << AFD_POLL_ABORT_BIT)
#define AFD_POLL_LOCAL_CLOSE_BIT        5
#define AFD_POLL_LOCAL_CLOSE            (1 << AFD_POLL_LOCAL_CLOSE_BIT)
#define AFD_POLL_CONNECT_BIT            6
#define AFD_POLL_CONNECT                (1 << AFD_POLL_CONNECT_BIT)
#define AFD_POLL_ACCEPT_BIT             7
#define AFD_POLL_ACCEPT                 (1 << AFD_POLL_ACCEPT_BIT)
#define AFD_POLL_CONNECT_FAIL_BIT       8
#define AFD_POLL_CONNECT_FAIL           (1 << AFD_POLL_CONNECT_FAIL_BIT)
#define AFD_POLL_QOS_BIT                9
#define AFD_POLL_QOS                    (1 << AFD_POLL_QOS_BIT)
#define AFD_POLL_GROUP_QOS_BIT          10
#define AFD_POLL_GROUP_QOS              (1 << AFD_POLL_GROUP_QOS_BIT)

#define AFD_NUM_POLL_EVENTS             11
#define AFD_POLL_ALL                    ((1 << AFD_NUM_POLL_EVENTS) - 1)


#define FSCTL_AFD_BASE                  FILE_DEVICE_NETWORK
#define _AFD_CONTROL_CODE(request,method) \
                ((FSCTL_AFD_BASE)<<12 | (request<<2) | method)
#define _AFD_REQUEST(ioctl) \
                ((((ULONG)(ioctl)) >> 2) & 0x03FF)
#define _AFD_BASE(ioctl) \
                ((((ULONG)(ioctl)) >> 12) & 0xFFFFF)


#define AFD_POLL                    9

#define IOCTL_AFD_POLL                    _AFD_CONTROL_CODE( AFD_POLL, METHOD_BUFFERED )

#define POLL_STATIC_SIZE 4

ULONG MSAFD_WSPPoll(WSAPOLLFD *fdArray, ULONG fds, int timeout)
{
	NTSTATUS status;
	ULONG error;
	PAFD_POLL_INFO pollInfo;
	ULONG pollBufferSize;
	PAFD_POLL_HANDLE_INFO pollHandleInfo;
	ULONG handleCount;
	ULONG i;
	ULONG n;
	HANDLE eventHandle;
	IO_STATUS_BLOCK ioStatusBlock;
	ULONG handlesReady;

	UCHAR pollinfo[sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*POLL_STATIC_SIZE];

	if (fdArray == NULL)
	{
		error = WSAEINVAL;
		goto exit;
	}

	if (fds == 0)
	{
		error = 0; //???
		goto exit;
	}

	pollBufferSize = sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*fds;
	if (pollBufferSize > (sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*POLL_STATIC_SIZE))
	{
		pollInfo = (AFD_POLL_INFO *)RtlAllocateHeap(GetProcessHeap(), 0, pollBufferSize);
		if (!pollInfo)
		{
			error = WSAENOBUFS;
			goto exit;
		}
		memset(pollInfo, 0, pollBufferSize);
	} else
	{
		pollInfo = (AFD_POLL_HANDLE_INFO*)&pollinfo;
		memset(pollInfo, 0, sizeof(pollinfo));
	}

	if (timeout < 0)
		pollInfo->Timeout = INT64_MAX;
	else	pollInfo->Timeout = timeout * -10000;

	handleCount = 0;
	ULONG maxnfds = 0;

	for (i = 0; i < fds; ++i)
	{
		fdArray[i].revents = 0;
		if (fdArray[i].fd == INVALID_HANDLE)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		if ( (fdArray[i].events & ~(POLLRDBAND|POLLRDNORM|POLLWRNORM)) != 0 )
		{
			error_code = WSAEINVAL;
			goto exit;
		} else
		{
			handleCount = maxnfds;
			BOOL found = FALSE;
			n = 0;

			pollHandleInfo = pollInfo->Handles;

			while (n < maxnfds)
			{
				if (pollHandleInfo->Handle == fdArray[i].fd)
				{
					found = TRUE;
					break;
				}
				++pollHandleInfo;
				++n;
			}
			pollHandleInfo->Handle = fdArray[i].fd;
			if ( (fdArray[i].events & POLLRDNORM) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_ACCEPT|AFD_POLL_RECEIVE;
			if ( (fdArray[i].events & POLLRDBAND) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_RECEIVE_EXPEDITED;
			if ( (fdArray[i].events & POLLWRNORM) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_CONNECT_FAIL|AFD_POLL_SEND; //++win10fix
			pollHandleInfo->PollEvents |= AFD_POLL_ABORT|AFD_POLL_DISCONNECT;

			if (!found)
				handleCount = ++maxnfds;
		}
	}
	
	if (handleCount == 0)
	{
		error = WSAEINVAL;
		goto exit;
	}

	pollInfo->NumberOfHandles = handleCount;
	pollInfo->Unique = FALSE;

	status = NtDeviceIoControlFile(pollInfo->Handles[0].Handle,
					NULL, //??? TODO
					NULL,
					NULL,
					&ioStatusBlock,
					IOCTL_AFD_POLL, //0x12024
					pollInfo, pollBufferSize,
					pollInfo, pollBufferSize);
	if (status == STATUS_PENDING)
	{
		//??? TODO
	}

	if (!NT_SUCCESS(status))
	{
		error = NtStatusToSocketError(status);
		goto exit;
	}

	for (i = 0; i < fds; ++i)
	{
		pollHandleInfo = pollInfo->Handles;

		for (n = 0; n < pollInfo->NumberOfHandles; ++n)
		{
			if (pollHandleInfo->Handle == fdArray[i].fd)
			{
				if ( (pollHandleInfo->PollEvents & (AFD_POLL_ACCEPT|AFD_POLL_RECEIVE)) != 0 &&
					(fdArray[i].events & POLLRDNORM) == POLLRDNORM )
				{
					fdArray[i].revents |= POLLRDNORM;
				}

				if ( (pollHandleInfo->PollEvents & AFD_POLL_RECEIVE_EXPEDITED) != 0 &&
					(fdArray[i].events & POLLRDBAND) == POLLRDBAND )
				{
					fdArray[i].revents |= POLLRDBAND;
				}

				if ( (pollHandleInfo->PollEvents & (AFD_POLL_CONNECT_FAIL|AFD_POLL_SEND)) != 0 && //++win10fix
					(fdArray[i].events & POLLWRNORM) != 0) // POLLOUT
					fdArray[i].revents |= POLLWRNORM; // POLLOUT

				if ( (pollHandleInfo->PollEvents & AFD_POLL_DISCONNECT) != 0 )
					fdArray[i].revents |= POLLHUP;

				if ( (pollHandleInfo->PollEvents & (AFD_POLL_CONNECT_FAIL|AFD_POLL_ABORT)) != 0 ) //++win10fix
					fdArray[i].revents |= POLLHUP|POLLERR;

				if ( (pollHandleInfo->PollEvents & AFD_POLL_LOCAL_CLOSE) != 0 )
					fdArray[i].revents |= POLLNVAL;
        		}
			++pollHandleInfo;
		}
	}

	handlesReady = 0;
	for (i = 0; i < fds; ++i)
	{
		if ( (fdArray[j].revents & (POLLRDBAND|POLLRDNORM|POLLWRNORM|POLLHUP)) != 0 )
			++handlesReady;
	}

	handlesReady = pollInfo->NumberOfHandles;
	
exit:
	if (pollInfo && pollInfo != &pollinfo)
		RtlFreeHeap(GetProcessHeap(), 0, pollInfo);

	SetLastError(error);
	return error ? SOCKET_ERROR : handlesReady;
}


int WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout)
{
	ULONG i;
	ULONG error = NO_ERROR;

	LPWSAPOLLFD pollInfo = NULL;

	if (fds == 0)
	{
		error = WSAEINVAL;
		goto exit;
	}

	ULONG pollInfoBufferSize = sizeof(WSAPOLLFD) * fds;
	pollInfo = HeapAlloc(GetProcessHeap(), 0, pollInfoBufferSize);
	if (!pollInfo)
	{
		error = WSAENOBUFS;
		goto exit;
	}
	memcpy(pollInfo, fdArray, pollInfoBufferSize);

	BOOL found = FALSE;
	SOCKET foundfd = INVALID_SOCKET;

	for (i = 0; i < fds; ++i)
	{
		SOCKET nativefd = INVALID_SOCKET;
		DWORD BytesReturned = 0;
		SOCKET fd = pollInfo[i].fd;
		if (fd == INVALID_SOCKET)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		if (WSAIoctl(fd, SIO_BSP_HANDLE_POLL, NULL, NULL, &nativefd, sizeof(nativefd), &BytesReturned, NULL, NULL) == SOCKET_ERROR)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		{
			if (!found)
			{
				foundfd = nativefd;
				found = TRUE;
			}
			pollInfo[i].fd = nativefd;
		}
	}
	
	if (!found)
	{
		error = WSAENOTSOCK;
		goto exit;
	}

	ULONG result = MSAFD_WSPPoll(pollInfo, fds, timeout);
	if (result == SOCKET_ERROR)
	{
		error = GetLastError();
		goto exit;
	}

	for (i = 0; i < fds; ++i)
	{
		fdArray[i].revents = pollInfo[i].revents;
	}
	HeapFree(GetProcessHeap(), 0, pollInfo);
	return result;
exit:
	if (pollInfo)
		HeapFree(GetProcessHeap(), 0, pollInfo);
	SetLastError(error);
	return SOCKET_ERROR;
}
