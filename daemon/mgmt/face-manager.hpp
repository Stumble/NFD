/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Named Data Networking Project
 * See COPYING for copyright and distribution information.
 */

#ifndef NFD_MGMT_FACE_MANAGER_HPP
#define NFD_MGMT_FACE_MANAGER_HPP

#include "common.hpp"
#include "face/face.hpp"
#include "face/local-face.hpp"
#include "mgmt/app-face.hpp"
#include "mgmt/manager-base.hpp"
#include "mgmt/config-file.hpp"
#include "mgmt/face-status-publisher.hpp"
#include "mgmt/notification-stream.hpp"
#include "fw/face-table.hpp"

#include <ndn-cpp-dev/management/nfd-control-parameters.hpp>
#include <ndn-cpp-dev/management/nfd-control-response.hpp>

namespace nfd {

const std::string FACE_MANAGER_PRIVILEGE = "faces";

class ProtocolFactory;
class NetworkInterfaceInfo;
class LocalFace;

class FaceManager : public ManagerBase
{
public:
  class Error : public ManagerBase::Error
  {
  public:
    Error(const std::string& what) : ManagerBase::Error(what) {}
  };

  /**
   * \throws FaceManager::Error if localPort is an invalid port number
   */

  FaceManager(FaceTable& faceTable,
              shared_ptr<InternalFace> face);

  virtual
  ~FaceManager();

  /** \brief Subscribe to a face management section(s) for the config file
   */
  void
  setConfigFile(ConfigFile& configFile);

  void
  onFaceRequest(const Interest& request);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  listFaces(const Interest& request);

PROTECTED_WITH_TESTS_ELSE_PRIVATE:

  void
  onValidatedFaceRequest(const shared_ptr<const Interest>& request);

  VIRTUAL_WITH_TESTS void
  createFace(const Interest& request,
             ControlParameters& parameters);

  VIRTUAL_WITH_TESTS void
  destroyFace(const Interest& request,
              ControlParameters& parameters);

  VIRTUAL_WITH_TESTS bool
  extractLocalControlParameters(const Interest& request,
                                ControlParameters& parameters,
                                ControlCommand& command,
                                shared_ptr<LocalFace>& outFace,
                                LocalControlFeature& outFeature);

  VIRTUAL_WITH_TESTS void
  enableLocalControl(const Interest& request,
                     ControlParameters& parambeters);

  VIRTUAL_WITH_TESTS void
  disableLocalControl(const Interest& request,
                      ControlParameters& parameters);

  void
  ignoreUnsignedVerb(const Interest& request);

  void
  addCreatedFaceToForwarder(const shared_ptr<Face>& newFace);

  void
  onCreated(const Name& requestName,
            ControlParameters& parameters,
            const shared_ptr<Face>& newFace);

  void
  onConnectFailed(const Name& requestName, const std::string& reason);

  void
  onAddFace(shared_ptr<Face> face);

  void
  onRemoveFace(shared_ptr<Face> face);

private:
  void
  onConfig(const ConfigSection& configSection, bool isDryRun, const std::string& filename);

  void
  processSectionUnix(const ConfigSection& configSection, bool isDryRun);

  void
  processSectionTcp(const ConfigSection& configSection, bool isDryRun);

  void
  processSectionUdp(const ConfigSection& configSection,
                    bool isDryRun,
                    const std::list<shared_ptr<NetworkInterfaceInfo> >& nicList);

  void
  processSectionEther(const ConfigSection& configSection,
                      bool isDryRun,
                      const std::list<shared_ptr<NetworkInterfaceInfo> >& nicList);

  /** \brief parse a config option that can be either "yes" or "no"
   *  \throw ConfigFile::Error value is neither "yes" nor "no"
   *  \return true if "yes", false if "no"
   */
  bool
  parseYesNo(const ConfigSection::const_iterator& i,
             const std::string& optionName,
             const std::string& sectionName);

private:
  typedef std::map< std::string/*protocol*/, shared_ptr<ProtocolFactory> > FactoryMap;
  FactoryMap m_factories;
  FaceTable& m_faceTable;
  FaceStatusPublisher m_statusPublisher;
  NotificationStream m_notificationStream;

  typedef function<void(FaceManager*,
                        const Interest&,
                        ControlParameters&)> SignedVerbProcessor;

  typedef std::map<Name::Component, SignedVerbProcessor> SignedVerbDispatchTable;
  typedef std::pair<Name::Component, SignedVerbProcessor> SignedVerbAndProcessor;

  typedef function<void(FaceManager*, const Interest&)> UnsignedVerbProcessor;

  typedef std::map<Name::Component, UnsignedVerbProcessor> UnsignedVerbDispatchTable;
  typedef std::pair<Name::Component, UnsignedVerbProcessor> UnsignedVerbAndProcessor;


  const SignedVerbDispatchTable m_signedVerbDispatch;
  const UnsignedVerbDispatchTable m_unsignedVerbDispatch;

  static const Name COMMAND_PREFIX; // /localhost/nfd/faces

  // number of components in an invalid signed command (i.e. should be signed, but isn't)
  // (/localhost/nfd/faces + verb + parameters) = 5
  static const size_t COMMAND_UNSIGNED_NCOMPS;

  // number of components in a valid signed command.
  // (see UNSIGNED_NCOMPS), 9 with signed Interest support.
  static const size_t COMMAND_SIGNED_NCOMPS;

  static const SignedVerbAndProcessor SIGNED_COMMAND_VERBS[];
  static const UnsignedVerbAndProcessor UNSIGNED_COMMAND_VERBS[];

  static const Name LIST_COMMAND_PREFIX;
  static const size_t LIST_COMMAND_NCOMPS;

  static const Name EVENTS_COMMAND_PREFIX;
};

inline bool
FaceManager::parseYesNo(const ConfigSection::const_iterator& i,
                        const std::string& optionName,
                        const std::string& sectionName)
{
  const std::string value = i->second.get_value<std::string>();
  if (value == "yes")
    {
      return true;
    }
  else if (value == "no")
    {
      return false;
    }

  throw ConfigFile::Error("Invalid value for option \"" +
                          optionName + "\" in \"" +
                          sectionName + "\" section");

}

inline void
FaceManager::ignoreUnsignedVerb(const Interest& request)
{
  // do nothing
}

} // namespace nfd

#endif // NFD_MGMT_FACE_MANAGER_HPP
