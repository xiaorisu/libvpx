class MotionField {
  int block_size;
  ArrayList<PVector> motion_field;

  MotionField(int block_size) {
    this.block_size = block_size;
    this.motion_field = new ArrayList<PVector>();
  }

  ArrayList<PVector> update(ArrayList<PVector> last_positions,
                            ArrayList<PVector> current_positions,
                            int[] render_list) {
    // build motion field
    motion_field = new ArrayList<PVector>();
    int r_num = height / block_size, c_num = width / block_size;
    // clear motion field
    for (int i = 0; i < r_num * c_num; i++)
      motion_field.add(new PVector(0, 0, 0));
    // accumate motion vector of pixel in each block
    for (int i = 0; i < height; i++)
      for (int j = 0; j < width; j++) {
        if (render_list[i * width + j] == -1) continue;
        PVector cur_pos = current_positions.get(render_list[i * width + j]);
        PVector last_pos = last_positions.get(render_list[i * width + j]);
        int idx = i / block_size * c_num + j / block_size;
        PVector mv = PVector.sub(last_pos, cur_pos);
        PVector acc_mv = motion_field.get(idx);
        motion_field.set(
            idx, new PVector(acc_mv.x + mv.x, acc_mv.y + mv.y, acc_mv.z + 1));
      }
    for (int i = 0; i < r_num * c_num; i++) {
      PVector mv = motion_field.get(i);
      if (mv.z > 0) {
        motion_field.set(i, new PVector(mv.x / mv.z, mv.y / mv.z, 0));
      }
    }
    return motion_field;
  }

  void show() {
    int r_num = height / block_size, c_num = width / block_size;
    for (int i = 0; i < r_num; i++)
      for (int j = 0; j < c_num; j++) {
        PVector mv = motion_field.get(i * c_num + j);
        float ox = j * block_size + 0.5f * block_size;
        float oy = i * block_size + 0.5f * block_size;
        stroke(255, 0, 0);
        line(ox, oy, ox + mv.x, oy + mv.y);
      }
  }
}
