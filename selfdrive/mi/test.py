import time
from selfdrive.df import df_wrapper
df_model = df_wrapper.get_wrapper()
df_model.init_model()
'''start = time.time()
for i in range(30):
 model_output = df_model.run_model(0.8653078153514447,
  0.46805728618371273,
  0.46805728618371273,
  0.28780443294609244,
  0.01075646532123655)
print(time.time() - startl)'''

df_input = [0]*54

#s = time.time()

for i in range(100):
  model_output = df_model.run_model_live_tracks(df_input)
  print("Model output: {}".format(model_output))


model_output = df_model.run_model_live_tracks(df_input)
print("Model output: {}".format(model_output))


#print(time.time()-s)
'''model_output = df_model.run_model(.3, 0.0, .3, .5)
print(model_output)
model_output = df_model.run_model(.05, .3, .4, .6)
print(model_output)
model_output = df_model.run_model(.7, .85, .55, .5)
print(model_output)'''