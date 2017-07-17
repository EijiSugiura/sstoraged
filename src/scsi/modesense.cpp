/**
   @file modesense.cpp
   @brief SCSI ModeSense handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: modesense.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include "common.h"

#ifdef HAVE_SCSI_SCSI_H
#include <scsi/scsi.h>
#else
#include "scsi.h"
#endif

#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "iscsi/protocol.h"
#include "scsi/generic.h"
#include "scsi/modesense.h"

ModeSenseHandler::ModeSenseHandler()
{}

bool ModeSenseParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != MODE_SENSE){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	/** @todo : Check SN */

	return true;
}

bool ModeSenseHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "MODE SENSE(6)");
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	ModeSenseParser cdb(reinterpret_cast<ModeSenseHeader*>(req.getSCB()));

	if(!(req.getFlags() & COMMAND_FLAG_READ) ||
	   req.getFlags() & COMMAND_FLAG_WRITE ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid MODE SENSE request");
		return false;
	}
	/** Check Final flag */
	if(!(req.isFinal())){
		LOG4CXX_WARN(logger, "Initiator MUST set final bit");
 		return false;
	}

	/** generate DataIn */
 	DataInGenerator reply;
	reply.setCmdStatus(GOOD);
	reply.setITT(req.getITT());
	/** Target Transfer Tag is not supplied 
	    @todo : support DataACK & SNACK */
	reply.setTTT(ISCSI_RSVD_TASK_TAG);
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();
	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());
	
	ModeSenseData data(reinterpret_cast<ModeParamHeader*>
			   (reply.reserveDataSegment(cdb.getAllocationLength())),
			   cdb.getAllocationLength());

	/** get Volume info. and set last LBA */
	iSCSIInitiatorPtr initiator = parent->initiator.get();
	LogicalVolumePtr lv = initiator->getLUN(req.getLUN().get());
	if(lv.get() == NULL){
		LOG4CXX_ERROR(logger, "Invalid LUN : " +
			      boost::lexical_cast<string>(req.getLUN().get()));
		return false;
	}

	if(!cdb.isDBD()){
		data.setBlockDescriptorLength(sizeof(ShortLBABlockDescriptor));
	}
	if(cdb.getAllocationLength() == data.HEADER_SIZE){
		/** header only */
		LOG4CXX_DEBUG(logger, "header only");
		data.setDesiredLength(0x1F);
		goto header_only;
	}
	if(!cdb.isDBD()){
		data.appendBlockDescriptor(lv);
	}

	switch(cdb.getPageCode()){
	case 0x00:
		break;
	case 0x02:
		data.appendDisconnectReconnect();
		break;
	case 0x03:
		data.appendFormatDevice();
		break;
	case 0x04:
		data.appendRigidDiskGeometry();
		break;
	case 0x08:
		data.appendCaching();
		break;
	case 0x0A:
		data.appendControl();
		break;
	case 0x1C:
		data.appendInformationalExceptionsCtrl();
		break;
	case 0x3F:
		data.appendDisconnectReconnect();
		data.appendFormatDevice();
		data.appendRigidDiskGeometry();
		data.appendCaching();
		data.appendControl();
		data.appendInformationalExceptionsCtrl();
		break;
	default:
		LOG4CXX_ERROR(logger, "Invalid MODE SENSE page code");
		return false;
	}
	if(data.getLength() > cdb.getAllocationLength()){
		LOG4CXX_WARN(logger, "Too small data segment for MODE SENSE(6) : " +
			     boost::lexical_cast<string>(data.getLength()) + "/" +
			     boost::lexical_cast<string>(data.getLimit()));
	}
 header_only:
	if(!reply.resetDlength(min(data.getLength(),
				   static_cast<size_t>(cdb.getAllocationLength()))))
		return false;
	/** set Residual Count */
	reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

bool ModeSenseData::appendBlockDescriptor(const LogicalVolumePtr lv)
{
	LOG4CXX_DEBUG(logger, "BlockDescriptor");
	size_t prev = total;
	total += sizeof(ShortLBABlockDescriptor);
	if(total > getLimit()){
		LOG4CXX_WARN(logger, "BlockDescriptorLength : Illegal length " +
			     boost::lexical_cast<string>(total) + "/" +
			     boost::lexical_cast<string>(getLimit()));
		return false;
	}
	if(!setLength(total)){
		LOG4CXX_WARN(logger, "BlockDescriptor : omitted ");
		return false;
	}

	ShortLBABlockDescriptor *desc =
		reinterpret_cast<ShortLBABlockDescriptor*>((uint8_t*)(data) + prev);

 	desc->setNumberOfLBA((lv->getTotalSize()>>32)?
 			     0xFFFFFFFF:lv->getTotalSize());
	return true;
}

bool ModeSenseData::appendDisconnectReconnect()
{
	LOG4CXX_DEBUG(logger, "DisconnectReconnect");
	struct DisconnectReconnect {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t	buffer_full_ratio;
		uint8_t	buffer_empty_ratio;
		uint8_t	bus_inactivity_limit[2];
		uint8_t	disconnect_time_limit[2];
		uint8_t	connect_time_limit[2];
		uint8_t	max_burst_size[2];
		uint8_t	reserve1[2];
		uint8_t	first_burst_size[2];
	} *mode_page;
	size_t prev = total;
	total += sizeof(DisconnectReconnect);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "DisconnectReconnect : omitted");
		return false;
	}
	mode_page = reinterpret_cast<DisconnectReconnect*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x02;
	mode_page->plength = 14;
	mode_page->buffer_full_ratio = 0x80;
	mode_page->buffer_empty_ratio = 0x80;
	mode_page->bus_inactivity_limit[1] = 10;	// 10[s]

	return true;
}

bool ModeSenseData::appendFormatDevice()
{
	LOG4CXX_DEBUG(logger, "FormatDevice");
	struct FormatDevice {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t	tracks_per_zone[2];
		uint8_t alternate_sector_per_zone[2];
		uint8_t alternate_tracks_per_zone[2];
		uint8_t alternate_tracks_per_lu[2];
		uint8_t sector_per_track[2];
		uint8_t data_bytes_per_physical_sector[2];
		uint8_t interleave[2];
		uint8_t track_skew_factor[2];
		uint8_t cylinder_skew_factor[2];
		uint8_t flags;
		uint8_t reserve1[3];
	} *mode_page;
	size_t prev = total;
	total += sizeof(FormatDevice);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "FormatDrive : omitted");
		return false;
	}
	mode_page = reinterpret_cast<FormatDevice*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x03;
	mode_page->plength = 22;
	mode_page->sector_per_track[0] = 1;			// 256bytes
	mode_page->data_bytes_per_physical_sector[0] = 2; 	// 512bytes
	mode_page->flags = 0x0c;				// ???

	return true;
}

bool ModeSenseData::appendRigidDiskGeometry()
{
	LOG4CXX_DEBUG(logger, "RigidDiskGeometry");
	struct RigidDiskGeometry {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t num_of_cylinders[3];
		uint8_t num_of_heads;
		uint8_t write_precompensation[3];
		uint8_t reduced_write_current[3];
		uint8_t device_step_rate[2];
		uint8_t landing_zone_cylinder[3];
		uint8_t reserved1;
		uint8_t rotational_offset;
		uint8_t reserved2;
		uint16_t medium_rotation_rate;
		uint8_t reserved3[2];
	} __attribute__ ((packed)) *mode_page;
	size_t prev = total;
	total += sizeof(RigidDiskGeometry);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "RigidDiskGeometry : omitted");
		return false;
	}
	mode_page = reinterpret_cast<RigidDiskGeometry*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x04;
	mode_page->plength = 22;
	hton24(mode_page->num_of_cylinders, 0x80);	// 128
	mode_page->num_of_heads = 0x40;			// 64
	mode_page->medium_rotation_rate = htons(15000); 	// 15000rpm

	return true;
}

bool ModeSenseData::appendCaching()
{
	LOG4CXX_DEBUG(logger, "Caching");
	struct Caching {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t flags1[2];
#ifdef SPC4_ENABLED
		uint8_t disable_prefetch_transfer_length[2];
		uint8_t min_prefetch[2];
		uint8_t max_prefetch[2];
		uint8_t max_prefetch_ceiling[2];
		uint8_t flags2;
		uint8_t num_of_cache_segments;
		uint8_t cache_segment_size[2];
		uint8_t reserved[4];
#endif
	} *mode_page;
	size_t prev = total;
	total += sizeof(Caching);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "Caching : omitted ");
		return false;
	}
	mode_page = reinterpret_cast<Caching*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x08;
	mode_page->flags1[0] = 0x14;	// DISC|WCE
#ifndef SPC4_ENABLED
	mode_page->plength = 2;
#else
	mode_page->plength = 18;
	*(uint16_t *)mode_page->disable_prefetch_transfer_length = 0xFFFF;
	*(uint16_t *)mode_page->max_prefetch = 0xFFFF;
	*(uint16_t *)mode_page->max_prefetch_ceiling = 0xFFFF;
	mode_page->flags2 = 0x80;	// FSW
	mode_page->num_of_cache_segments = 20;
#endif
	return true;
}

bool ModeSenseData::appendControl()
{
	LOG4CXX_DEBUG(logger, "Control");
	struct Control {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t flags[4];
		uint8_t obsolute[2];
		uint8_t busy_timeout_period[2];
		uint16_t extended_selftest_completion_time;
	} __attribute__ ((packed)) *mode_page;
	size_t prev = total;
	total += sizeof(Control);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "Control : omitted");
		return false;
	}
	mode_page = reinterpret_cast<Control*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x0A;
	mode_page->plength = 10;
	mode_page->flags[0] = 0x02;	// GLTSD
	// *(uint16_t*)mode_page->busy_timeout_period = 0xFFFF;	// SPC4
	mode_page->extended_selftest_completion_time = htons(500);

	return true;
}

bool ModeSenseData::appendInformationalExceptionsCtrl()
{
	LOG4CXX_DEBUG(logger, "InformationalExceptionsControl");
	struct InformationalExceptionsCtrl {
		uint8_t	pcode;
		uint8_t	plength;
		uint8_t flags[2];
		uint8_t interval_timer[4];
		uint8_t report_count[4];
	} *mode_page;
	size_t prev = total;
	total += sizeof(InformationalExceptionsCtrl);

	if(!setLength(total)){
		LOG4CXX_WARN(logger, "InformationalExceptionsCtrl : omitted");
		return false;
	}
	mode_page = reinterpret_cast<InformationalExceptionsCtrl*>((uint8_t*)(data) + prev);

	mode_page->pcode = 0x1C;
	mode_page->plength = 10;
	mode_page->flags[0] = 0x08;	// DEXCPT

	return true;
}
