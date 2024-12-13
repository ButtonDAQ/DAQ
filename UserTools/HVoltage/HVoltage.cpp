#include "HVoltage.h"

HVoltage::HVoltage(): Tool() {}

void HVoltage::connect() {
  std::stringstream ss;
  std::string string;
  for (int i = 0; ; ++i) {
    uint32_t vme;
    ss.str({});
    ss << "hv_" << i << "_vme";
    if (!m_variables.Get(ss.str(), string)) break;
    ss.str({});
    ss << string;
    ss >> std::hex >> vme;

    uint32_t usb = 0;
    ss.str({});
    ss << "hv_" << i << "_usb";
    m_variables.Get(ss.str(), usb);

    info()
      << "connecting to high voltage board V6534 "
      << i
      << " (vme = "
      << std::hex << vme << std::dec
      << ", usb = "
      << usb
      << ")..."
      << std::flush;
    boards.emplace_back(
        caen::V6534(
          {
            .link = CAENComm_USB,
            .arg  = usb,
            .vme  = vme << 16
          }
        )
    );
    info() << "success" << std::endl;

    if (m_verbose >= 3) {
      auto& hv = boards.back();
      info()
        << "model: " << hv.model() << ' ' << hv.description() << '\n'
        << "serial number: " << hv.serial_number() << '\n'
        << "firmware version: " << hv.vme_fwrel()
        << std::endl;
    };
  };
};

void HVoltage::configure() {
  std::stringstream ss;
  for (int i = 0; i < boards.size(); ++i)
    for (int channel = 0; channel < 6; ++channel) {
      float voltage = 0;
      ss.str({});
      ss << "hv_" << i << "_ch_" << channel;
      if (m_variables.Get(ss.str(), voltage)) {
        boards[i].set_voltage(channel, voltage);
        boards[i].set_power(channel, true);
      } else
        boards[i].set_power(channel, false);
    };
};

bool HVoltage::Initialise(std::string configfile, DataModel& data) {
  InitialiseTool(data);
  InitialiseConfiguration(configfile);

  if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

  connect();
  configure();

  ExportConfiguration();
  return true;
};

bool HVoltage::Execute() {
  return true;
};

bool HVoltage::Finalise() {
  for (auto& board : boards)
    for (int channel = 0; channel < 6; ++channel)
      board.set_power(channel, false);
  return true;
};
