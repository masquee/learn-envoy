
# 使用 demo 配置
envoy -c config/envoy-demo.yaml

# 覆盖默认配置
 envoy -c config/envoy-demo.yaml --config-yaml "$(cat config/envoy-override.yaml)"

# 校验 envoy 配置文件
 envoy --mode validate -c config/envoy-demo.yaml


# 使用 grpc 配置
envoy -c config/envoy-dynamic-control-plane-demo.yaml

 