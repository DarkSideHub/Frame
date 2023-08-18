
#include "ExtensionSystem.h"

#include "moc_ExtensionSystem.cpp"
namespace ExtensionSystem {
std::mutex ObjectMutex;

std::string PLUGIN_METADATA = "MetaData";
std::string PLUGIN_NAME = "Name";
std::string PLUGIN_VERSION = "Version";
std::string PLUGIN_REQUIRED = "Required";
std::string ARGUMENTS = "Arguments";
std::string ARGUMENT_NAME = "Name";
std::string ARGUMENT_PARAMETER = "Parameter";
std::string ARGUMENT_DESCRIPTION = "Description";
std::string DEPENDENCIES = "Dependencies";
std::string DEPENDENCY_NAME = "Name";
std::string DEPENDENCY_VERSION = "Version";
std::string DEPENDENCY_TYPE = "Type";
std::string DEPENDENCY_TYPE_SOFT = "optional";
std::string DEPENDENCY_TYPE_HARD = "required";
std::string DEPENDENCY_TYPE_TEST = "test";
std::string CATEGORY = "Category";

bool SPEC::Load() noexcept {
  if (STATE::Resolved != State && STATE::Loaded != State) {
    return false;
  }
  if (STATE::Loaded == State) {
    return true;
  }
  //
  if (!Loader.load()) {
    std::cout << Name.toStdString()
              << "加载错误:" << Loader.errorString().toStdString() << std::endl;
    return false;
  }
  PluginObject.reset(qobject_cast<IPlugin*>(Loader.instance()));
  if (nullptr == PluginObject) {
    Loader.unload();
    return false;
  }
  State = STATE::Loaded;
  return true;
};

bool SPEC::Initialize() noexcept {
  if (STATE::Loaded != State && STATE::Initialized != State) {
    return false;
  }
  if (STATE::Initialized == State) {
    return true;
  }
  if (nullptr == PluginObject) {
    return false;
  }
  if (!PluginObject->Initialize(Arg)) {
    return false;
  }
  State = STATE::Initialized;
  return true;
}

bool SPEC::Kill() noexcept {
  if (nullptr == PluginObject) {
    return true;
  }
  PluginObject.reset();
  State = STATE::Deleted;
  return true;
}

bool SPEC::Run() noexcept {
  if (STATE::Initialized != State && STATE::Running != State) {
    return false;
  }
  if (STATE::Running == State) {
    return true;
  }
  if (nullptr == PluginObject) {
    return false;
  }
  if (!PluginObject->Run()) {
    return false;
  }
  State = STATE::Running;
  return true;
}
StopFlag SPEC::Stop() noexcept {
  if (nullptr == PluginObject) {
    return StopFlag::SynchronousStop;
  }
  State = STATE::Stopped;
  return PluginObject->Stop();
}

bool SPEC::Read(const QString& filePath) noexcept {
  State = STATE::Invalid;
  Loader.setFileName(filePath);
  if (Loader.fileName().isEmpty()) {
    return false;
  }
  QJsonObject MetaData;
  // TODO后期加上
  /*

  std::string mIID =
  Loader.metaData().value(QLatin1String("IID")).tostdString();
  if(mIID!=IID){
    return false;
  }

  QJsonDocument jsDoc(Loader.metaData());

  QFile file1("sa.json");
  file1.open(QIODevice::ReadWrite);
  file1.write(jsDoc.toJson());
  file1.close();
*/
  try {
    MetaData =
        Loader.metaData().value(QLatin1String(PLUGIN_METADATA)).toObject();
  } catch (...) {
    return false;
  }
  QJsonValue value;
  value = MetaData.value(QLatin1String(PLUGIN_NAME));
  if (value.isUndefined() || !value.isString()) {
    return false;
  }
  SetName(value.toString());
  value = MetaData.value(QLatin1String(PLUGIN_VERSION));
  if (value.isUndefined() || !value.isString() ||
      !IsValidVersion(value.toString())) {
    return false;
  }
  SetVersion(value.toString());
  value = MetaData.value(QLatin1String(CATEGORY));
  if (value.isUndefined() || !value.isString()) {
    return false;
  }
  SetCategory(value.toString());
  value = MetaData.value(QLatin1String(DEPENDENCIES));
  if (value.isUndefined() || !value.isArray()) {
    return false;
  }

  for (const QJsonValue& v : value.toArray()) {
    if (!v.isObject()) {
      return false;
    }
    QJsonObject dependencyObject = v.toObject();
    DEPENDENCY dep;
    QJsonValue Name = dependencyObject.value(QLatin1String(DEPENDENCY_NAME));
    QJsonValue Version =
        dependencyObject.value(QLatin1String(DEPENDENCY_VERSION));
    QJsonValue Type = dependencyObject.value(QLatin1String(DEPENDENCY_TYPE));

    if (Name.isUndefined() || !Name.isString()) {
      return false;
    }
    if ((Version.isUndefined() || !Version.isString() ||
         !IsValidVersion(Version.toString()))) {
      return false;
    }
    if ((Type.isUndefined() || !Type.isString())) {
      Dependencies.push_back(
          std::make_tuple(Name.toString(), Version.toString(), TYPE::Optional));
    } else {
      if (QLatin1String(DEPENDENCY_TYPE_HARD) == Type.toString().toLower()) {
        Dependencies.push_back(std::make_tuple(
            Name.toString(), Version.toString(), TYPE::Required));
      } else if (QLatin1String(DEPENDENCY_TYPE_SOFT) ==
                 Type.toString().toLower()) {
        Dependencies.push_back(std::make_tuple(
            Name.toString(), Version.toString(), TYPE::Optional));

      } else {
        Dependencies.push_back(
            std::make_tuple(Name.toString(), Version.toString(), TYPE::Test));
      }
    }
  }
  // TODO 还需要解析参数
  State = STATE::Read;
  return true;
}

namespace PluginManager {

std::shared_ptr<QObject> PLUGIN_MANAGE::GetObject(
    const QString& mName) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  for (auto Spec : PluginSpecs) {
    if (Spec->GetName() == mName) {
      return Spec->GetPlugin();
    }
  }
  return nullptr;
}
template <typename T>
std::shared_ptr<T> PLUGIN_MANAGE::GetObject() noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  for (auto mObject : Objects) {
    // TODO 如果一个插件类有多个实例，会报错
    if (std::is_same<T, decltype(mObject.get())>::value) {
      return mObject;
    }
  }
  return nullptr;
}

bool PLUGIN_MANAGE::AddObject(std::shared_ptr<IPlugin> mObject) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  if (nullptr != mObject &&
      (std::end(Objects) == std::find(begin(Objects), end(Objects), mObject))) {
    Objects.push_back(mObject);
    // emit objectChange(mObject);
    return true;
  }
  // emit objectChange(mObject);
  return false;
}

bool PLUGIN_MANAGE::RemoveObject(std::shared_ptr<IPlugin> mObject) noexcept {
  std::lock_guard<std::mutex> lock(ObjectMutex);
  if (nullptr != mObject) {
    auto it = std::find(begin(Objects), end(Objects), mObject);
    if (std::end(Objects) != it) {
      Objects.erase(it);
      // emit objectChange(mObject);
      return true;
    }
  }
  // emit objectChange(mObject);
  return false;
}

bool PLUGIN_MANAGE::Load() noexcept {
  // 首先读取插件信息，求解插件之间的依赖，并建立加载顺序
  PluginSpecs.clear();
  PluginCategory.clear();
  QDir PluginsDir(QGuiApplication::applicationDirPath());
  std::string sa = PluginsDir.absolutePath().toStdString();
  if ("windows" == PLATFORM) {
    if (PluginsDir.dirName().toLower() == "debug" ||
        PluginsDir.dirName().toLower() == "release") {
      PluginsDir.cdUp();
    }
  } else if ("apple" == PLATFORM) {
    if (PluginsDir.dirName() == "MacOS") {
      PluginsDir.cdUp();
      PluginsDir.cdUp();
      PluginsDir.cdUp();
    }
  }
  PluginsDir.cd("plugins");
  const QStringList Entries = PluginsDir.entryList(QDir::Files);
  for (const QString& FileName : Entries) {
    auto Spec = std::make_shared<SPEC>();
    if (!Spec->Read(PluginsDir.absoluteFilePath(FileName))) {
      continue;
    }
    PluginCategory[Spec->GetCategory()].push_back(Spec);
    PluginSpecs.push_back(Spec);
  }
  if (!Resolve()) {
    return false;
  }
  // 循环依赖检查
  // Queue中元素的顺序就是加载的顺序
  std::vector<std::shared_ptr<SPEC>> Queue;
  for (auto Spec : PluginSpecs) {
    std::vector<std::shared_ptr<SPEC>> Circularity;
    if (!CircularityCheck(Spec, Queue, Circularity)) {
      // TODO 弹窗提示有插件有循环依赖
      std::cout << "有循环依赖" << std::endl;
    }
  }

  // 加载插件时，判断依赖项的状态是不是已加载
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Loaded)) {
      std::cout << "加载插件失败" << std::endl;
    }
  }
  // 初始化插件：其实就是给插件传参
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Initialized)) {
      std::cout << "初始化插件失败" << std::endl;
    }
  }
  for (const auto Spec : Queue) {
    if (!EnterTargetState(Spec, STATE::Running)) {
      std::cout << "初始化插件失败" << std::endl;
    }
  }
  return true;
}

bool PLUGIN_MANAGE::Resolve() noexcept {
  for (auto iter : PluginSpecs) {
    if (STATE::Resolved == iter->GetState()) {
      iter->SetState(STATE::Read);
    }
    if (STATE::Read != iter->GetState()) {
      return false;
    }
    iter->DependencySpecsClear();
    auto Dependencies = iter->GetDependency();
    for (DEPENDENCY Dependency : Dependencies) {
      // 根据依赖项的名字找到依赖项实例对应的指针
      std::shared_ptr<SPEC> Spec = PLUGIN_MANAGE::Find(Dependency);
      if (!Spec) {
        if (TYPE::Required == std::get<2>(Dependency)) {
          return false;
        } else {
          continue;
        }
      }
      iter->DependencySpecsInsert(Dependency, Spec);
    }
    iter->SetState(STATE::Resolved);
  }
  return true;
};

std::shared_ptr<SPEC> PLUGIN_MANAGE::Find(
    const DEPENDENCY& Dependency) noexcept {
  for (std::shared_ptr<SPEC> Spec : PluginSpecs) {
    if (Spec->GetName() == std::get<0>(Dependency) &&
        Spec->GetVersion() == std::get<1>(Dependency) &&
        Spec->GetType() == std::get<2>(Dependency)) {
      return Spec;
    }
  }
  return nullptr;
}

bool PLUGIN_MANAGE::CircularityCheck(
    std::shared_ptr<SPEC> Spec, std::vector<std::shared_ptr<SPEC>>& Queue,
    std::vector<std::shared_ptr<SPEC>>& CircularityCheckQueue) noexcept {
  if (Queue.end() != std::find(begin(Queue), end(Queue), Spec)) {
    // 已经在队列中就直接返回
    return true;
  }
  if (CircularityCheckQueue.end() != std::find(begin(CircularityCheckQueue),
                                               end(CircularityCheckQueue),
                                               Spec)) {
    // 检测到循环依赖
    return false;
  }
  CircularityCheckQueue.push_back(Spec);
  if (Spec->GetState() == STATE::Read || Spec->GetState() == STATE::Invalid) {
    return false;
  }
  auto DependencySpecs = Spec->GetDependencySpecs();
  for (const auto iter : DependencySpecs) {
    if (std::get<2>(iter.first) == TYPE::Test) {
      continue;
    }
    if (!CircularityCheck(iter.second, Queue, CircularityCheckQueue)) {
      return false;
    }
  }
  Queue.push_back(Spec);
  return true;
};

bool PLUGIN_MANAGE::EnterTargetState(const std::shared_ptr<SPEC> Spec,
                                     STATE TargetState) noexcept {
  switch (TargetState) {
    case STATE::Loaded: {
      if (Spec->GetState() != STATE::Resolved) {
        return false;
      }
      // 检查依赖项是否已经加载
      auto DependencySpecs = Spec->GetDependencySpecs();

      for (const auto iter : DependencySpecs) {
        if (std::get<2>(iter.first) != TYPE::Required) {
          continue;
        }
        if (iter.second->GetState() != TargetState) {
          return false;
        }
      }
      if (!Spec->Load()) {
        return false;
      }
      // 插件sendSignal与PM的receive绑定
      QObject::connect(Spec->GetPlugin().get(), &IPlugin::sendSignal, this,
                       &PLUGIN_MANAGE::Receive);
      // PM的sendSignal与插件的receive绑定
      /*QObject::connect(this, &PLUGIN_MANAGE::sendSignal, Spec->GetPlugin(),
                        &IPlugin::Receive);*/
      return AddObject(Spec->GetPlugin());
    }
    case STATE::Initialized: {
      if (Spec->GetState() != STATE::Loaded) {
        return false;
      }
      // 检查依赖项是否已经加载
      auto DependencySpecs = Spec->GetDependencySpecs();
      for (const auto iter : DependencySpecs) {
        if (std::get<2>(iter.first) != TYPE::Required) {
          continue;
        }
        if (iter.second->GetState() != TargetState) {
          return false;
        }
      }
      return Spec->Initialize();
    }
    case STATE::Running: {
      if (Spec->GetState() != STATE::Initialized) {
        return false;
      }
      return Spec->Run();
    }
    case STATE::Deleted: {
      return Spec->Kill();
    }
    case STATE::Stopped: {
      Spec->Stop();
      return true;
    }
    default:
      break;
  }
  return true;
};

void PLUGIN_MANAGE::Receive(MSG Msg) noexcept {
  QString receiver = std::get<1>(Msg);
  QString log;

  if ("Window" == receiver) {
    log = "测试结束";
    emit sendLog(log, LOGLEVEL::Info);
    return;
  }
  for (auto Spec : PluginSpecs) {
    if (Spec->GetName() == receiver) {
      auto mPlugin = Spec->GetPlugin();
      if (nullptr == mPlugin) {
        log = "转发消息时目标插件" + receiver + "没有实例";
        emit sendLog(log, LOGLEVEL::Error);
        return;
      }
      log = "从" + std::get<0>(Msg) + "向" + receiver + "转发消息成功";
      emit sendLog(log, LOGLEVEL::Info);
      mPlugin->Receive(Msg);
      return;
    }
  }
  log = "转发消息时没有找到目标插件" + receiver;
  emit sendLog(log, LOGLEVEL::Error);
  return;
}

bool PLUGIN_MANAGE::UnLoad(const QString& Path) noexcept { return true; }
bool PLUGIN_MANAGE::UnLoad() noexcept { return true; }
};  // namespace PluginManager
};  // namespace ExtensionSystem
