#include "dynamic_follow.h"
using namespace std;

std::unique_ptr<zdl::SNPE::SNPE> snpe;

float *output;

zdl::DlSystem::Runtime_t checkRuntime()
{
    static zdl::DlSystem::Version_t Version = zdl::SNPE::SNPEFactory::getLibraryVersion();
    static zdl::DlSystem::Runtime_t Runtime;
    std::cout << "SNPE Version: " << Version.asString().c_str() << std::endl; //Print Version number
    if (zdl::SNPE::SNPEFactory::isRuntimeAvailable(zdl::DlSystem::Runtime_t::GPU)) {
        Runtime = zdl::DlSystem::Runtime_t::GPU;
    } else {
        Runtime = zdl::DlSystem::Runtime_t::CPU;
    }
    return Runtime;
}

void initializeSNPE(zdl::DlSystem::Runtime_t runtime) {
  std::unique_ptr<zdl::DlContainer::IDlContainer> container;
  container = zdl::DlContainer::IDlContainer::open("/data/openpilot/selfdrive/df/live_tracksvHIGHWAY.dlc");
  //printf("loaded model\n");
  int counter = 0;
  zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
  snpe = snpeBuilder.setOutputLayers({})
                      .setRuntimeProcessor(runtime)
                      .setUseUserSuppliedBuffers(false)
                      .setPerformanceProfile(zdl::DlSystem::PerformanceProfile_t::HIGH_PERFORMANCE)
                      .build();
}


std::unique_ptr<zdl::DlSystem::ITensor> loadInputTensor(std::unique_ptr<zdl::SNPE::SNPE> &snpe, std::vector<float> inputVec) {
  std::unique_ptr<zdl::DlSystem::ITensor> input;
  const auto &strList_opt = snpe->getInputTensorNames();
  if (!strList_opt) throw std::runtime_error("Error obtaining Input tensor names");
  const auto &strList = *strList_opt;

  const auto &inputDims_opt = snpe->getInputDimensions(strList.at(0));
  const auto &inputShape = *inputDims_opt;

  input = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(inputShape);
  std::copy(inputVec.begin(), inputVec.end(), input->begin());

  return input;
}

float returnOutput(const zdl::DlSystem::ITensor* tensor) {
  float op = *tensor->cbegin();
  return op;
}

float returnOutputMulti(const zdl::DlSystem::ITensor* tensor) {
  vector<float> outputs;
  for (auto it = tensor->cbegin(); it != tensor->cend(); ++it ){
    float op = *it;
    outputs.push_back(op);
    }
  float gas = outputs.at(0);
  float brake = outputs.at(1);
  if (gas > brake) {
    return gas;
  } else if (brake > gas){
    return -brake;
  } else {
    return 0.0;
  }
}

zdl::DlSystem::ITensor* executeNetwork(std::unique_ptr<zdl::SNPE::SNPE>& snpe,
                    std::unique_ptr<zdl::DlSystem::ITensor>& input) {
  static zdl::DlSystem::TensorMap outputTensorMap;
  snpe->execute(input.get(), outputTensorMap);
  zdl::DlSystem::StringList tensorNames = outputTensorMap.getTensorNames();

  const char* name = tensorNames.at(0);  // only should the first
  auto tensorPtr = outputTensorMap.getTensor(name);
  return tensorPtr;
}

extern "C" {
  void init_model(){
      zdl::DlSystem::Runtime_t runt=checkRuntime();
      initializeSNPE(runt);
  }

  float run_model(float v_ego, float v_lead, float x_lead, float a_lead){
    std::vector<float> inputVec;
    inputVec.push_back(v_ego);
    inputVec.push_back(v_lead);
    inputVec.push_back(x_lead);
    inputVec.push_back(a_lead);

    std::unique_ptr<zdl::DlSystem::ITensor> inputTensor = loadInputTensor(snpe, inputVec);
    zdl::DlSystem::ITensor* oTensor = executeNetwork(snpe, inputTensor);
    return returnOutput(oTensor);
  }

  float run_model_live_tracks_multi(float inputData[54]){
      int size = 54;
      std::vector<float> inputVec;
      for (int i = 0; i < size; i++ ) {
        inputVec.push_back(inputData[i]);
      }

      std::unique_ptr<zdl::DlSystem::ITensor> inputTensor = loadInputTensor(snpe, inputVec);
      zdl::DlSystem::ITensor* oTensor = executeNetwork(snpe, inputTensor);
      return returnOutputMulti(oTensor);
  }

  float run_model_live_tracks(float inputData[54]){
      int size = 54;
      std::vector<float> inputVec;
      for (int i = 0; i < size; i++ ) {
        inputVec.push_back(inputData[i]);
      }

      std::unique_ptr<zdl::DlSystem::ITensor> inputTensor = loadInputTensor(snpe, inputVec);
      zdl::DlSystem::ITensor* oTensor = executeNetwork(snpe, inputTensor);
      return returnOutput(oTensor);
  }

int main(){
  std::cout << "hello";
  return 0;
}

}
