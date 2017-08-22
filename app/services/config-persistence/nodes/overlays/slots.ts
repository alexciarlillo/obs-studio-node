import { ArrayNode } from '../array-node';
import { SceneItem, Scene } from '../../../scenes';
import { VideoService } from '../../../video';
import { SourcesService } from '../../../sources';
import { Inject } from '../../../../util/injector';
import { ImageNode } from './image';
import { TextNode } from './text';
import { WebcamNode } from './webcam';
import { VideoNode } from './video';

interface ISchema {
  name: string;

  x: number;
  y: number;

  // These values are normalized for a 1920x1080 base resolution
  scaleX: number;
  scaleY: number;

  content: ImageNode | TextNode | WebcamNode | VideoNode;
}

interface IContext {
  assetsPath: string;
  scene: Scene;
}

export class SlotsNode extends ArrayNode<ISchema, IContext, SceneItem> {

  schemaVersion = 1;

  @Inject()
  videoService: VideoService;

  @Inject()
  sourcesService: SourcesService;

  getItems(context: IContext) {
    return context.scene.getItems().slice().reverse();
  }


  saveItem(sceneItem: SceneItem, context: IContext): ISchema {
    const details = {
      name: sceneItem.name,
      x: sceneItem.x / this.videoService.baseWidth,
      y: sceneItem.y / this.videoService.baseHeight,
      scaleX: sceneItem.scaleX / this.videoService.baseWidth,
      scaleY: sceneItem.scaleY / this.videoService.baseHeight
    };

    if (sceneItem.type === 'image_source') {
      const content = new ImageNode();
      content.save({ sceneItem, assetsPath: context.assetsPath });

      return { ...details, content };
    } else if (sceneItem.type === 'text_gdiplus') {
      const content = new TextNode();
      content.save({ sceneItem, assetsPath: context.assetsPath });

      return { ...details, content };
    } else if (sceneItem.type === 'dshow_input') {
      const content = new WebcamNode();
      content.save({ sceneItem, assetsPath: context.assetsPath });

      return { ...details, content };
    } else if (sceneItem.type === 'ffmpeg_source') {
      const content = new VideoNode();
      content.save({ sceneItem, assetsPath: context.assetsPath });

      return { ...details, content };
    }

    return null;
  }


  loadItem(obj: ISchema, context: IContext) {
    let sceneItem: SceneItem;

    if (obj.content instanceof WebcamNode) {
      const existingWebcam = this.sourcesService.sources.find(source => {
        return source.type === 'dshow_input';
      });

      if (existingWebcam) {
        sceneItem = context.scene.addSource(existingWebcam.sourceId);
      } else {
        sceneItem = context.scene.createAndAddSource(obj.name, 'dshow_input');
      }

      this.adjustPositionAndScale(sceneItem, obj);

      obj.content.load({
        sceneItem,
        assetsPath: context.assetsPath,
        existing: existingWebcam !== void 0
      });

      return;
    }

    if (obj.content instanceof ImageNode) {
      sceneItem = context.scene.createAndAddSource(obj.name, 'image_source');
    } else if (obj.content instanceof TextNode) {
      sceneItem = context.scene.createAndAddSource(obj.name, 'text_gdiplus');
    } else if (obj.content instanceof VideoNode) {
      sceneItem = context.scene.createAndAddSource(obj.name, 'ffmpeg_source');
    }

    this.adjustPositionAndScale(sceneItem, obj);
    obj.content.load({ sceneItem, assetsPath: context.assetsPath });
  }


  adjustPositionAndScale(item: SceneItem, obj: ISchema) {
    item.setPositionAndScale(
      obj.x * this.videoService.baseWidth,
      obj.y * this.videoService.baseHeight,
      obj.scaleX * this.videoService.baseWidth,
      obj.scaleY * this.videoService.baseHeight
    );
  }


  normalizedScale(scale: number) {
    return scale * (1920 / this.videoService.baseWidth);
  }


  denormalizedScale(scale: number) {
    return scale / (1920 / this.videoService.baseWidth);
  }

}
