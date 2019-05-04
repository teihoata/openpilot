import zmq
import selfdrive.messaging as messaging
from selfdrive.services import service_list
import selfdrive.kegman_conf as kegman
import subprocess
from common.basedir import BASEDIR

class Phantom():
  def __init__(self):
    context = zmq.Context()
    self.poller = zmq.Poller()
    self.phantom_Data_sock = messaging.sub_sock(context, service_list['phantomData'].port, conflate=True, poller=self.poller)
    self.phantomData = None
    self.data = {"status": False, "speed": 0.0}
    if (BASEDIR == "/data/openpilot") and (not kegman.get("UseDNS") or not kegman.get("UseDNS")):
      self.mod_sshd_config()

  def update(self):
    for socket, event in self.poller.poll(0):
      if socket is self.phantom_Data_sock:
        self.phantomData = messaging.recv_one(socket).phantomData

    if self.phantomData:
      self.data = {"status": self.phantomData.status, "speed": self.phantomData.speed, "angle": self.phantomData.angle, "time": self.phantomData.time}
    else:
      self.data = {"status": False, "speed": 0.0}

  def mod_sshd_config(self):  # this disables dns lookup when connecting to EON to speed up commands from phantom app, reboot required
    sshd_config_file = "/system/comma/usr/etc/ssh/sshd_config"
    result = subprocess.check_call(["mount", "-o", "remount,rw", "/system"])  # mount /system as rw so we can modify sshd_config file
    if result == 0:
      with open(sshd_config_file, "r") as f:
        sshd_config = f.read()
      if "UseDNS no" not in sshd_config:
        if sshd_config[-1:]!="\n":
          use_dns = "\nUseDNS no\n"
        else:
          use_dns = "UseDNS no\n"
        with open(sshd_config_file, "w") as f:
          f.write(sshd_config + use_dns)
        kegman.save({"UseDNS": True})
      else:
        kegman.save({"UseDNS": True})
      subprocess.check_call(["mount", "-o", "remount,ro", "/system"])  # remount system as read only
    else:
      kegman.save({"UseDNS": False})